mod lexer;

use lexer::Lexer;

fn main() {
    let source = std::fs::read_to_string("examples/test.jcl").unwrap();

    println!("source len: {}", source.len());

    for token in Lexer::from_source(&source) {
        println!("token: {:?}", token);
    }
}
