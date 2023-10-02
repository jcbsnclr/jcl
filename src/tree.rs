//! A [`TokenTree`] is a semi-structured view of the source code, grouping tokens between brackets

use crate::lexer::{BracketKind, Lexer, Span, Token, TokenKind};

use std::fmt;

/// The contents of a [`TokenTree`]
#[derive(Debug, Clone)]
pub enum TokenTreeData<'a> {
    /// A group of [`TokenTree`]s between 2 brackets
    Group(Option<BracketKind>, Vec<TokenTree<'a>>),
    /// An individual [`Token`]
    Token(Token<'a>),
}

/// A semi-structured view of the source file, grouping [`Token`]s between brackets
#[derive(Debug, Clone)]
pub struct TokenTree<'a> {
    data: TokenTreeData<'a>,
    span: Span,
    view: &'a str,
}

struct StackFrame<'a> {
    kind: Option<BracketKind>,
    span: Span,
    body: Vec<TokenTree<'a>>,
}

#[derive(Debug, Clone)]
pub enum ParserErrorKind {
    UnexpectedDelimiter(BracketKind),
    ExpectedDelimiter(BracketKind),
}

#[derive(Debug, Clone)]
pub struct ParserError {
    pub span: Span,
    pub kind: ParserErrorKind,
}

impl ParserError {
    pub fn new(span: Span, kind: ParserErrorKind) -> ParserError {
        ParserError { span, kind }
    }
}

/// Consumes [`Token`]s from a [`Lexer`] and produces a [`TokenTree`] as it's end result
struct Stack<'a> {
    stack: Vec<StackFrame<'a>>,
}

impl<'a> Stack<'a> {
    fn new() -> Stack<'a> {
        Stack {
            stack: vec![StackFrame {
                kind: None,
                span: Span::from_start(),
                body: vec![],
            }],
        }
    }

    fn push(&mut self, tree: TokenTree<'a>) {
        let StackFrame { body, span, .. } = self
            .stack
            .iter_mut()
            .last()
            .expect("token tree stack exhausted");

        span.join_with(tree.span());
        body.push(tree);
    }

    fn open(&mut self, kind: BracketKind, span: Span) {
        self.stack.push(StackFrame {
            kind: Some(kind),
            span,
            body: vec![],
        });
    }

    fn close(&mut self, found: BracketKind, span: Span) -> Result<(), ParserError> {
        let StackFrame {
            body,
            kind: expected,
            span,
        } = self.stack.pop().expect("token tree stack exhausted");

        if expected != Some(found) {
            Err(ParserError::new(
                span,
                ParserErrorKind::UnexpectedDelimiter(found),
            ))
        } else {
            self.push(TokenTree {
                data: TokenTreeData::Group(Some(found), body),
                span,
                view: "",
            });

            Ok(())
        }
    }

    fn finalise(mut self, lexer: &Lexer<'a>) -> TokenTree<'a> {
        let StackFrame { body, span, .. } = self.stack.pop().expect("token tree stack exhausted");
        let view = lexer.extract(span);

        TokenTree {
            data: TokenTreeData::Group(None, body),
            span,
            view,
        }
    }

    fn feed(&mut self, lexer: &mut Lexer<'a>) {
        while let Some(token) = lexer.next() {
            self.push(TokenTree {
                data: TokenTreeData::Token(token),
                span: token.span(),
                view: token.view(),
            });
        }
    }
}

impl<'a> TokenTree<'a> {
    pub fn data(&self) -> &TokenTreeData<'a> {
        &self.data
    }

    pub fn span(&self) -> Span {
        self.span
    }

    pub fn view(&self) -> &'a str {
        self.view
    }

    pub fn from_source(source: &'a str) -> TokenTree<'a> {
        let mut lexer = Lexer::from_source(source);
        let mut stack = Stack::new();

        stack.feed(&mut lexer);

        stack.finalise(&lexer)
    }
}

struct TreeDisplay<'a>(usize, &'a TokenTree<'a>);

impl<'a> fmt::Display for TreeDisplay<'a> {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        for _ in 0..self.0 * 2 {
            write!(fmt, " ")?;
        }

        match self.1.data {
            TokenTreeData::Token(token) => write!(fmt, "{}", token)?,

            TokenTreeData::Group(kind, body) => {}

            _ => unimplemented!(),
        }

        Ok(())
    }
}
