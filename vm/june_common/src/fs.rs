use pathdiff::diff_paths;
use regex::Regex;
use std::env;
use std::path::Path;
use walkdir::WalkDir;

pub fn search(
  dir: String,
  predicate: impl Fn(&walkdir::DirEntry) -> bool,
) -> Vec<String> {
  let mut files = Vec::new();
  for entry in WalkDir::new(dir)
    .follow_links(true)
    .into_iter()
    .filter_map(|entry| {
      if let Ok(entry) = entry {
        if predicate(&entry) {
          Some(entry)
        } else {
          None
        }
      } else {
        None
      }
    })
  {
    if let Ok(path) = entry.path().canonicalize() {
      files.push(path.to_str().unwrap().to_string());
    }
  }

  files
}

pub fn search_regex(dir: String, re: &str) -> Vec<String> {
  let re = Regex::new(re).unwrap();
  search(dir, |ent| re.is_match(ent.file_name().to_str().unwrap()))
}

pub fn relative_path(path: &Path) -> Option<String> {
  Some(
    diff_paths(path.canonicalize().ok()?, env::current_dir().ok()?)?
      .to_str()?
      .to_string()
  )
}
