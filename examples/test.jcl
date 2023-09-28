# print "Hello, World!" to console
echo "Hello, World!";

# prompt user for their age and parse it as an integer
let age: [prompt "What is your age?" | parse_int $];

if [lt age 18] then: {
	echo "You are too young to buy alcohol";
} else: {
	echo "You are old enough to buy alcohol";
};
