//! embed-rs - Multi-language Embedded Examples
//!
//! This crate provides examples of embedded systems development in multiple languages.
//!
//! For the actual firmware implementations, see:
//! - `/firmware` - C implementation
//! - `/firmware-cpp` - C++ implementation
//! - `/firmware-rust` - Rust implementation
//!
//! This is just a placeholder demonstrating Rust project structure.

fn main() {
    println!("embed-rs: Multi-language Embedded Systems Examples");
    println!("====================================================");
    println!();
    println!("This repository contains three implementations:");
    println!("  1. C       - Traditional bare-metal (firmware/)");
    println!("  2. C++     - Modern C++17 with OOP (firmware-cpp/)");
    println!("  3. Rust    - Safe embedded Rust (firmware-rust/)");
    println!();
    println!("See README.md for build instructions.");
    println!();
    println!("To build the Rust firmware:");
    println!("  cd firmware-rust && cargo build --release");
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}
