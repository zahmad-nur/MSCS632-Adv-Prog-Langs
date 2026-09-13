//Section 3 Memory Demo for Rust

struct Data {
    values: Vec<i32>,
}

// Borrows Data immutably; no ownership transfer, no copy
fn sum_borrowed(d: &Data) -> i32 {
    d.values.iter().sum()
}

fn main() {
    let data = Data { values: (0..1000).collect() };
    println!("Sum: {}", sum_borrowed(&data));

    // The commented block below does not compile 
    // The borrow checker rejects it
    // at compile time because `dangling_ref` would outlive `local`.
    //
    // fn make_dangling() -> &i32 {
    //     let local = 5;
    //     &local   // ERROR: `local` does not live long enough
    // }

    println!("data dropped automatically at end of scope, no free() call needed");
} // <- `data` goes out of scope here; Rust auto-inserts the equivalent of `drop(data)`