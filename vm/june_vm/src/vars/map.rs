use crate::vars::{Value, Native, NativeFeature};
use hashbrown::HashMap;
use std::borrow::Cow;

#[derive(Debug, Clone)]
pub struct VarMap {
  inner: HashMap<String, Value>,
}

impl VarMap {
  pub fn new() -> Self {
    Self {
      inner: HashMap::new(),
    }
  }
}

impl Default for VarMap {
  fn default() -> Self {
    Self::new()
  }
}

impl Native for VarMap {
  fn is_feature_supported(&self, feature: NativeFeature) -> bool {
    matches!(feature, NativeFeature::AttrBased)
  }

  fn attr_exists(&self, name: &str) -> bool {
    self.inner.contains_key(name)
  }

  fn attr_set(&mut self, name: &str, value: Value) -> Result<(), Cow<'static, str>> {
    self.inner.insert(name.to_string(), value);
    Ok(())
  }

  fn attr_get(&self, name: &str) -> Result<&Value, Cow<'static, str>> {
    match self.inner.get(name) {
      Some(value) => Ok(value),
      None => Err(Cow::Borrowed("no such attribute")),
    }
  }
}
