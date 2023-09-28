use std::iter::Peekable;
use std::str::CharIndices;

/// A [Span] represents a slice of a source string
#[derive(Debug, Clone, Copy)]
pub struct Span {
    start: usize,
    end: usize,
}

/// The different kinds of brackets
#[derive(Debug, Clone, Copy)]
pub enum BracketKind {
    Paren,
    Square,
    Brace,
}

/// The different types of [Token] found in the language
#[derive(Debug, Clone, Copy)]
pub enum TokenKind {
    Comment,
    Whitespace,

    Colon,
    Semicolon,

    Open(BracketKind),
    Close(BracketKind),

    Identifier,
    String,
    Number,

    /// Catch-all for any characters that are not handled by the other cases
    Other(char),
}

/// A [Token] describes a piece of a source string
#[derive(Debug, Clone, Copy)]
pub struct Token<'a> {
    /// The [Span] the token occupies
    span: Span,
    /// What the token represents
    kind: TokenKind,
    /// The text the token describes
    view: &'a str,
}

/// The [Lexer] is an iterator over [Token]s within a source string
pub struct Lexer<'a> {
    source: &'a str,
    stream: Peekable<CharIndices<'a>>,
}

impl<'a> Lexer<'a> {
    /// Produce a [Lexer] from a given source string
    pub fn from_source(source: &'a str) -> Lexer<'a> {
        Lexer {
            source,
            stream: source.char_indices().peekable(),
        }
    }
}

impl<'a> Iterator for Lexer<'a> {
    type Item = Token<'a>;

    fn next(&mut self) -> Option<Token<'a>> {
        unimplemented!()
    }
}
