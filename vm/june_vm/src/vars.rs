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
  }
}

pub struct Var {
  pub value: Value,
  pub src_id: usize,
  pub idx: usize,
  pub info: VarInfo,
  ref_count: usize,
}

/// Represents the bytecode instruction span of a function
#[derive(Debug, Clone)]
pub struct FuncSpan {
  pub start: usize,
  pub end: usize,
}

#[derive(Debug, Clone)]
pub enum Value {
  Integer(isize),
  Float(f64),
  String(String),
  Vector(Vec<Box<Value>>),
  Func(FuncSpan),
  Native(Box<dyn Native>),
}

/// Native feature types, used to implement functionality
/// such as calling functions, getting attributes, etc.
pub enum NativeFeature {
  Callable,
  AttrBased,
  LoadAsRef,
}

#[allow(unused_variables)]
pub trait Native: Debug + DynClone {
  /// feature sanity checks
  /// returns true if a feature is supported
  fn is_feature_supported(&self, feature: NativeFeature) -> bool { false }
  /// function call
  /// `thing()`
  fn call(&self, state: State, args: &[Value]) -> Result<Value, Cow<'static, str>> { Err("Native type not callable".into()) }
  /// attribute existence check
  /// `"attr" in thing`
  fn attr_exists(&self, name: &str) -> bool { false }
  /// attribute access
  /// `thing.attr`
  fn attr_get(&self, name: &str) -> Result<&Value, Cow<'static, str>> { Err("Native type not attr based".into()) }
  /// attribute assignment
  /// `thing.attr = value`
  fn attr_set(&mut self, name: &str, value: Value) -> Result<(), Cow<'static, str>> { Err("Native type not attr based".into()) }
}
clone_trait_object!(Native);

impl Var {
  pub fn new(src_id: usize, idx: usize, info: VarInfo, value: Value) -> Self {
    Self {
      value,
      src_id,
      idx,
      info,
      ref_count: 1,
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
      }
    }
  }

  pub fn call(&self, state: State, args: &[Value]) -> Result<Value, Cow<'static, str>> {
    if let Value::Native(native) = &self.value {
      native.call(state, args)
    } else {
      Err(Cow::Borrowed("not callable"))
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
    self.ref_count += 1;
  }

  pub fn dref(&mut self) {
    self.ref_count -= 1;
  }

  pub fn ref_count(&self) -> usize {
    self.ref_count
  }
}

#[macro_export]
macro_rules! var_cast {
  ($var:expr, $ty:ty) => {
    match $var.downcast_ref::<$ty>() {
      Some(v) => v,
      None => None,
    }
  };
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
}
