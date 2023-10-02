mod lexer;
mod tree;

use lexer::Lexer;

fn main() {
    let source = std::fs::read_to_string("examples/test.jcl").unwrap();
    let tree = tree::TokenTree::from_source(&source);

    dbg!(tree);
}
