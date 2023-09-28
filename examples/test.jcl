# print "Hello, World!" to console. commands are delimited by semicolons.

			# some whitespace

echo "Hello, World!";

# prompt user for their age and parse it as an integer
#
# the text between `[ ... ]` is known as an "inline" batch; it evaluates
# to the result of the commands inside
# 
# the syntax of `identifier: value` describes a "pair", which is simply 
# a key-value pair passed into a command. 
# 
# here we decalare a variable called age and give it a value which is the result
# of a command. the let command is used to define variables
#
# the pipe operator is used to take the return value of the previous command and
# bind it to `$`, and then execute the following command. this is to provide better ergonomics
# for chaining function calls than e.g. `baz [bar [foo "123"]]`, enabling you to do 
# `foo "123" | bar $ | baz $`. 
# (though perhaps in this instance it doesn't aid ergonomics too much :^))
let age: [prompt "What is your age?" | parse_int $];

# just like let, `if` makes use of pairs. `if` is not special, it is like any other command in
# the language, which is why we re-use the pair syntax here, to allow us to name a given part 
# of the command (in this case, the successful and unsuccessful branches).
#
# the text between `{ ... }` is called a "batch", and is a list of commands.
# it is not demonstrated in this instance, but the result of a batch, inline or not, is the 
# result of the last command in the batch. if the last command in a batch ends in a semicolon,
# then an empty command is inserted at the end of the batch, and nothing is returned
if [lt age 18] then: {
	echo "You are too young to buy alcohol";
} else: {
	echo "You are old enough to buy alcohol";
};
