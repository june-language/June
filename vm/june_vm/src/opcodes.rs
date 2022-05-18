pub enum OpCodes {
  Create,
  Store,
  Load,
  Unload,
  Jump,
  // if the value is true, pop
  JumpIfTrue(bool),
  JumpIfFalse(bool),
  JumpIfNil,
  BodyTill,
  MakeFunc,
  AllocBlock,
  DeallocBlock,
  Call,
  MemCall,
  Attr,
  Return,
  Continue,
  Break,
  PushLoop,
  PopLoop,
  PushJump,
  PushJumpNil,
  PopJump,
}

pub enum OpDataType {
  Int(isize),
  Float(f64),
  String(String),
  Ident(String),
  Size(isize),
  Bool(bool),
  Nil
}
