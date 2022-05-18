#![allow(non_upper_case_globals)]

pub mod map;

use bitflags::bitflags;
use dyn_clone::{DynClone, clone_trait_object};
use std::borrow::Cow;
use std::fmt::Debug;

use crate::state::State;

bitflags! {
  pub struct VarInfo: u32 {
    const Callable  = 0b00000001;
    const AttrBased = 0b00000010;
    const LoadAsRef = 0b00000100;
    const Unmanaged = 0b00001000;
  }
}

pub struct Var<'a> {
  pub value: Value<'a>,
  pub src_id: usize,
  pub idx: usize,
  pub info: VarInfo,
  ref_count: usize,
}

pub struct NativeTypeId {
  pub name: Option<String>,
  pub id: u64,
}

impl NativeTypeId {
  pub const fn of<T: ?Sized + 'static>(name: Option<String>) -> NativeTypeId {
    NativeTypeId { id: std::intrinsics::type_id::<T>(), name }
  }
}

/// Represents the bytecode instruction span of a function
#[derive(Debug, Clone)]
pub struct FuncSpan {
  pub start: usize,
  pub end: usize,
}

pub type NativeFn<'a> = fn(&'a mut State, &[Value]) -> Result<Value<'a>, String>;

/// Represents a function
#[derive(Debug, Clone)]
pub enum Func<'a> {
  Native(NativeFn<'a>),
  Bytecode(FuncSpan),
}

#[derive(Debug, Clone)]
pub enum Value<'a> {
  Nil,
  Integer(isize),
  Float(f64),
  String(String),
  Boolean(bool),
  Vector(Vec<Box<Value<'a>>>),
  Func(Func<'a>),
  Native(Box<dyn Native>),
}

/// Native feature types, used to implement functionality
/// such as calling functions, getting attributes, etc.
pub enum NativeFeature {
  Callable,
  AttrBased,
  LoadAsRef,
  Unmanaged,
}

#[allow(unused_variables)]
pub trait Native: Debug + DynClone {
  /// feature sanity checks
  /// returns true if a feature is supported
  fn is_feature_supported(&self, feature: NativeFeature) -> bool { false }
  /// function call
  /// `thing()`
  fn call<'a>(&self, state: State, args: &[Value<'a>]) -> Result<Value<'a>, Cow<'static, str>> { Err("Native type not callable".into()) }
  /// attribute existence check
  /// `"attr" in thing`
  fn attr_exists(&self, name: &str) -> bool { false }
  /// attribute access
  /// `thing.attr`
  fn attr_get<'a>(&self, name: &str) -> Result<&Value<'a>, Cow<'static, str>> { Err("Native type not attr based".into()) }
  /// attribute assignment
  /// `thing.attr = value`
  fn attr_set<'a>(&mut self, name: &str, value: Value<'a>) -> Result<(), Cow<'static, str>> { Err("Native type not attr based".into()) }
  /// type id for serialization
  fn type_id(&self) -> NativeTypeId;
}
clone_trait_object!(Native);

impl<'a> Var<'a> {
  pub fn new(src_id: usize, idx: usize, info: VarInfo, value: Value<'a>) -> Self {
    Self {
      value,
      src_id,
      idx,
      info,
      ref_count: 1,
    }
  }

  pub fn type_id(&self) -> NativeTypeId {
    match self.value {
      Value::Nil => NativeTypeId::of::<()>(Some("nil".into())),
      Value::Integer(_) => NativeTypeId::of::<isize>(Some("int".into())),
      Value::Float(_) => NativeTypeId::of::<f64>(Some("float".into())),
      Value::String(_) => NativeTypeId::of::<String>(Some("string".into())),
      Value::Boolean(_) => NativeTypeId::of::<bool>(Some("bool".into())),
      Value::Vector(_) => NativeTypeId::of::<Vec<Box<Value>>>(Some("Vec".into())),
      Value::Func(_) => NativeTypeId::of::<Func>(Some("Func".into())),
      Value::Native(ref native) => native.type_id(),
    }
  }

  pub fn is_feature_supported(&self, feature: NativeFeature) -> bool {
    if let Value::Native(native) = &self.value {
      native.is_feature_supported(feature)
    } else {
      match feature {
        NativeFeature::Callable => self.info.contains(VarInfo::Callable),
        NativeFeature::AttrBased => self.info.contains(VarInfo::AttrBased),
        NativeFeature::LoadAsRef => self.info.contains(VarInfo::LoadAsRef),
        NativeFeature::Unmanaged => self.info.contains(VarInfo::Unmanaged),
      }
    }
  }

  pub fn call(&self, state: State, args: &[Value]) -> Result<Value, Cow<'static, str>> {
    match &self.value {
      Value::Native(native) => native.call(state, args),
      Value::Vector(vec) => {
        assert!(cfg!(callable_vector), "callable vectors not enabled");

        if args.len() != 1 {
          return Err("callable indexing must contain only one argument".into());
        }

        let idx = match &args[0] {
          Value::Integer(idx) => *idx,
          _ => return Err("callable indexing must contain an integer".into()),
        };

        if idx >= vec.len().try_into().unwrap() || idx < 0 {
          return Err("callable indexing out of bounds".into());
        }

        let v = &vec[idx as usize];
        
        Ok(*v.clone())
      }
      _ => Err(Cow::Borrowed("not callable")),
    }
  }

  pub fn attr_exists(&self, name: &str) -> bool {
    if let Value::Native(native) = &self.value {
      native.attr_exists(name)
    } else {
      false
    }
  }

  pub fn attr_get(&self, name: &str) -> Result<&Value, Cow<'static, str>> {
    if let Value::Native(native) = &self.value {
      native.attr_get(name)
    } else {
      Err(Cow::Borrowed("not attr based"))
    }
  }

  pub fn attr_set(&mut self, name: &str, value: Value) -> Result<(), Cow<'static, str>> {
    if let Value::Native(native) = &mut self.value {
      native.attr_set(name, value)
    } else {
      Err(Cow::Borrowed("not attr based"))
    }
  }

  pub fn iref(&mut self) {
    // if the variable is unmanaged must be manually freed
    if self.info.contains(VarInfo::Unmanaged) { return; }

    self.ref_count += 1;
  }

  pub fn dref(&mut self) {
    if self.info.contains(VarInfo::Unmanaged) { return; }

    self.ref_count -= 1;
  }

  pub fn ref_count(&self) -> usize {
    self.ref_count
  }
}

#[macro_export]
macro_rules! var_ref {
  ($var:ident) => {
    $var.iref();
  };
}

#[macro_export]
macro_rules! var_deref {
  ($var:ident) => {
    $var.dref();
    if $var.ref_count() == 0 {
      drop($var);
    }
  };

  ($var:ident; unmanaged) => {
    // only used in `Memory.drop` to force drop the value
    $var.value = Value::Nil;
    $var.info.remove(VarInfo::Unmanaged); // allow it to be cleaned up later

    // its important that we don't drop the value
    // until it is completely cleaned up
  };
  
  ($var:ident; forced) => {
    // intended for use when a variable is left undropped yet managed
    $var.info.remove(VarInfo::Unmanaged); // allow it to be cleaned up
    $crate::var_deref!($var); // deref the var, drops if ref_count == 0
  };
}
