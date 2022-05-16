use crate::vars::Var;

pub struct State {


  pub exit_called: bool,
  pub exec_stack_count_exceeded: bool,
  pub exit_code: isize,
  pub exec_stack_count: usize,
  pub exec_stack_max: usize,

  // fails: VMFailStack,
  // src_stack: SrcStack,
  // stack: VMStack,

  pub tru: Var,
  pub fls: Var,
  pub nil: Var,

  // dylib: DylibMap,
  // src_args: Vec<String>,


}
