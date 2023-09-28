use std::iter::Peekable;
use std::str::CharIndices;

/// A [`Span`] represents a slice of a source string
#[derive(Debug, Clone, Copy)]
pub struct Span {
    start: usize,
    end: usize,
}

impl Span {
    /// Create a new [`Span`] over 2 positions
    fn new(start: usize, end: usize) -> Span {
        Span { start, end }
    }

    /// Create a new [`Span`] over one byte
    fn starting_at(start: usize) -> Span {
        Span::new(start, start + 1)
    }

    /// Extend the [`Span`] by the UTF-8 encoded length of a given `char`
    fn extend_by(&mut self, c: char) {
        self.end = self.end + c.len_utf8();
    }

    /// Get a [`std::ops::Range`] representing the [`Span`]
    pub fn as_range(self) -> std::ops::Range<usize> {
        self.start..self.end
    }
}

/// The different kinds of brackets
#[derive(Debug, Clone, Copy)]
pub enum BracketKind {
    Paren,
    Square,
    Brace,
}

/// The different types of [`Token`] found in the language
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

/// A [`Token`] describes a piece of a source string
#[derive(Debug, Clone, Copy)]
pub struct Token<'a> {
    /// The [`Span`] the token occupies
    span: Span,
    /// What the token represents
    kind: TokenKind,
    /// The text the token describes
    view: &'a str,
}

/// The [`Lexer`] is an iterator over [`Token`]s within a source string
pub struct Lexer<'a> {
    source: &'a str,
    stream: Peekable<CharIndices<'a>>,
}

impl<'a> Lexer<'a> {
    /// Produce a [`Lexer`] from a given source string
    pub fn from_source(source: &'a str) -> Lexer<'a> {
        Lexer {
            source,
            stream: source.char_indices().peekable(),
        }
    }

    /// Extend a [`Span`] while the stream matches a predicate
    fn take_while(&mut self, span: &mut Span, pred: impl Fn(char) -> bool) {
        // consume stream while char matches predicate
        while let Some((_, c)) = self.stream.next_if(|&(_, c)| pred(c)) {
            span.extend_by(c);
        }
    }

    /// Produce a [`Token`] representing a given [`Span`] of text, with a given [`TokenKind`]
    ///
    /// # Panics
    /// This function will panic if the end of the [`Span`] extends past the end of the
    /// source string.
    fn produce_token(&self, kind: TokenKind, span: Span) -> Token<'a> {
        Token {
            span,
            kind,
            view: &self.source[span.as_range()],
        }
    }

    /// Produce a [`Token`] of a given [`TokenKind`] while the stream matches a predicate.
    /// It returns `None` if the first char it consumes does not match the predicate.
    fn produce_while(&mut self, kind: TokenKind, pred: impl Fn(char) -> bool) -> Option<Token<'a>> {
        let &(start, _) = self.stream.peek()?;
        let mut span = Span::starting_at(start);

        self.take_while(&mut span, pred);

        Some(self.produce_token(kind, span))
    }
}

impl<'a> Iterator for Lexer<'a> {
    type Item = Token<'a>;

    fn next(&mut self) -> Option<Token<'a>> {
        let &(_, c) = self.stream.peek()?;

        match c {
            c if c.is_whitespace() => {
                self.produce_while(TokenKind::Whitespace, char::is_whitespace)
            }

            c if rules::identifier_start(c) => {
                self.produce_while(TokenKind::Identifier, rules::identifier_body)
            }

            '#' => self.produce_while(TokenKind::Comment, |c| c != '\n'),

            c => unimplemented!("{c}"),
        }
    }
}

// rules for the lexer
mod rules {
    /// The start of an identifier. Can be any valid Unicode alphabetic character, or an underscore
    pub fn identifier_start(c: char) -> bool {
        c.is_alphabetic() || c == '_'
    }

    /// The body of an identifier. Can be any valid [`identifier_start`], or any valid Unicode numeric character
    pub fn identifier_body(c: char) -> bool {
        identifier_start(c) || c.is_numeric()
    }
}
