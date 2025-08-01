// rudimentary JSON metrics summarizer (expects array of numbers in file)
use std::fs::File;
use std::io::{self, Read};
use serde_json::Value;

fn main() -> io::Result<()> {
    let mut buf = String::new();
    File::open("cleaned.json")?.read_to_string(&mut buf)?;
    let v: Value = serde_json::from_str(&buf).expect("invalid JSON");
    if let Some(arr) = v.as_array() {
        let nums: Vec<f64> = arr.iter().filter_map(|x| x.as_f64()).collect();
        let count = nums.len();
        let sum: f64 = nums.iter().sum();
        let mean = if count > 0 { sum / count as f64 } else { 0.0 };
        println!("count: {}", count);
        println!("sum: {:.4}", sum);
        println!("mean: {:.4}", mean);
    } else {
        eprintln!("expected JSON array of numbers");
        std::process::exit(1);
    }
    Ok(())
}
