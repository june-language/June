use crate::vars::Var;
use hashbrown::HashMap;
use libloading::Library;

/// Native init and deinit functions
pub type NativeInit = fn(&mut State, usize, usize) -> bool;
pub type NativeDeinit = fn() -> ();

/// These maps should have the same keys mapped to the same values
type DylibMap = HashMap<String, Library>;
type DeinitMap = HashMap<String, NativeDeinit>;

pub struct State<'a> {


  pub exit_called: bool,
  pub exec_stack_count_exceeded: bool,
  pub exit_code: isize,
  pub exec_stack_count: usize,
  pub exec_stack_max: usize,

  // fails: VMFailStack,
  // src_stack: SrcStack,
  // stack: VMStack,

  pub tru: Var<'a>, // Value::Boolean(true)
  pub fls: Var<'a>, // Value::Boolean(false)
  pub nil: Var<'a>, // Value::Nil

  pub dylib: DylibMap,
  pub deinit: DeinitMap,
  pub src_args: Var<'a>, // Value::Vector  
}
