# 240 Final Project

### Log Entries
~~2020-04-05 18:11 >> Well, here I am. Writing algorithms for the final project when I am still not even done with Perfect Pet. (Well, I got done with the base assignment.) I do not know if I am going to do this in C++ or Java (the Java implementation seems to be more 230, as there is little inheritance going on... maybe I will find out how difficult C++ object serialization is...)~~
>> If this ends up being a Java project then... I don't know. Maybe I can just wait until Tuesday before I try to mess with this.

2020-04-12 16:05 >> You know what, forget about that. I'm doing it in C++. And I will do it in C++ the RIGHT WAY. That means heap storage is king!
I mean, what is serialization even? I can just write it all in "plain" text to a file!
I did say I was going to use encryption. The easiest such algorithm seems to be RSA. That means I need a class for `BigNumber`s!

2020-04-13 12:47 >> Making rapid progress. I just got the subtraction and comparison done. Division is going to be a challenge; might need to get out the whiteboard for that.
Multiplication is slow (a 25x25 calculation looped 10000 times is taking just under one second), so I am considering using the Karatsuba algorithm for numbers above, say, 6 digits.

2020-04-14 16:58 >> I changed the mutliplication algorithm and it is much faster now. Long division is almost there; next comes modulo and modular exponentiation!

2020-04-14 17:39 >> Ugh. Division is WAY too slow (about 0.07s for a 1000/500). I think I need to optimize my `singleDigitDivide` function.

2020-04-15 19:46 >> I feel like a member of the Ministry of Truth. I now must scrub all references to `uint8_t` and `256` and the like, and change them to `uint16_t` and `65536`. For some strange reason, things are not working the way I want them to.

2020-04-25 16:22 >> I should have put my log entries in a separate file. I didn't write any for the past 10 days because the algorithm file got too long. If I decide to be a 230 TA for next semester, can someone please tell me how to grade someone else's algorithm file?
16:24 >> A couple days ago I discovered that we have access to `sqlite3` so of course I am using that. This morning I discovered we can `#include <openssl/...>`, but I think I learned quite a bit from struggling to build my own security systems. I did replace my SHA256 because it was giving wrong hashes.

## bigNumber
So according to my research, I need to do algorithms on very large numbers, much larger than even a 64-bit integer.

For this reason I am making my own class for large numbers. It stores numbers as an array of "digits" in base 2^16.
The culmination of this class will be the `modExp` function, which will compute `mod(a^b, c)`. Hopefully it is not too much time for large numbers.

But before I get there, I need some more basic methods...

Constructors:
`()`
- Basically, construct a zero.
- Set the digits to a single-element array consisting of only a 0.
- Set `numDigits` to 1.
`(uint16_t*, int)`
- I do not trust the source of that array!
- Allocate a new array of size the integer.
- For-loop copy everything over.
- Set `numDigits` to the int.
`(int)`
- Useful for constructing relatively small numbers.
- First, if the given num is < 0, throw an error.
- Create a temporary array buffer of size 8.
- Set `numDigits` to `0` for now.
- Create an int `valueLeft`, set to the parameter.
- As long as `valueLeft` is nonzero...
    - Get its value modulo the BASE. (This can be sped up with the bitwise operator `& (BASE - 1)`.)
    - Convert that to a `uint16_t` and store it in position `numDigits`.
    - Increment `numDigits`.
    - Chop off the lowest 16 bits of `valueLeft`. (Again, I can speed this up with the `>> 16` operator.)
- Now, copy from the `buffer` into `digits`.
- The number is constructed successfully.
`(std::string)`
- Used to parse a BigNumber given a HEXADECIMAL string.
- Each "digit" is going to represent 2 characters. Annoyingly they are in reverse order...
- First, get the `c_str` version of the input string.
- Also get the length, store into an int.
- The length must be even; if not, throw an error.
- If the length is 0...
    - Do what we did to construct a zero, then return immediately.
- Now, we know we need one digit for every 4 characters. Compute that size and allocate the array.
- Make 2 index variables: `d` (for digits) and `c` (for characters). Start `d` at `numDigits - 1` and `c` at 0.
- Loop while `c` is less than the string length:
    - (OK, extracting a hex value from 4 characters is surprisingly difficult!)
    - Create a `stringstream`.
    - Throw in the characters in order (that is, `[c]`, then `[c+1]`, etc).
    - Then create a `std::string`.
    - `getline`
    - Call `std::stoi` on the resultant string. I do not intend to catch errors.
    - Finally, we have an integer! Convert it to a `uint16_t` and plop it into position `[d]` of digits.
    - Advance `c` by 4
    - Reduce `d` by 1
- wow... that I guess in some way prepares me for what lies ahead
- but hey, we are done...
**copy constructor**
- Assign `numDigits` to the number of digits in `other`
- Use a for loop to copy each digit over.
Overloading `=` is done exactly the same way; in fact I can just copy the code.

Also on the agenda:
Two trivial methods `getDigit` and `getNumDigits`.
Also, `deflate()`:
- Create a variable `firstNonZero` set to `numDigits`. (It will decrease immediately once it enters the while loop.)
- Make a keep-going loop.
    - Decrease `firstNonZero`.
    - If the digit at `firstNonZero` is actually not 0, do not keep going.
    - If `firstNonZero` is 0, do not keep going. (This is so that we do not end up with an empty array.)
- `firstNonZero` will now be 1 less than the number of digits. Update `numDigits` to reflect.
- Now we need to do a bit of gymnastics here...
- Now, allocate a buffer of size the new `numDigits`.
- Copy the digits into the buffer.
- Now, create a pointer to hold the location of the old digit array.
- Then, assign the POINTER `digits` to that buffer.

I also have an `isZero()` function that loops over the digits and returns `false` if it finds any non-zero digits.

One last thing for debugging...
`dumpDigits()`:
- Basically, in a for loop, `cout` each digit with commas in between.

All right, time for some arithmetic!
Let's start with `+`:
- Get the number of digits of each input.
- Use a `?:` conditional assignment to get the larger number.
- Create a `uint16_t` buffer of that size + 1.
- Fill the buffer with 0's.
- Then loop again, this time only to `maxDigits`.
    - Get, as an **int**, the corresponding digits of A, B, and the buffer.
    - Add them all together.
    - Store the least significant 16 bits into the buffer.
    - Then, add any carry (i.e. `>> 16`) into the buffer in the _next_ position.
- Finally, construct a BigNumber with the buffer and the correct number of digits.
Now, let's handle multiplication.
For this, we need 2 helper methods: multiplication by a power of the base, and multiplication by a number less than the base.
The latter can be done with an operator overload.

`operator*(BigNumber a, uint16_t b)`:
- Do things a lot like for addition, but with some changes.
- First, only get the number of digits of `a`.
- Create a buffer of size `N+1`.
- Fill the buffer with 0s.
- Convert the given multiplier to an `int`.
- Start a for loop over the digits:
    - Convert the digit in slot `i` to an `int`.
    - Convert the carry in slot `i` to an `int`.
    - Do the expected `d*b + c`.
    - Convert the smallest 16 bits, and store it in slot `i`.
    - Convert any other bits (which will be < BASE), and store them in slot `i+1`.
- When done, construct a BigNumber from the buffer.

Lastly, before I can do the "real" multiplication, I need a static method that will shift a `BigNumber` by a certain number of digits. I guess I can make both directions; it can only help. Positive will multiply by 256^n, negative will divide by that.
- First, get the number of digits.
- Then check if `-places` (which is positive iff `places` is negative) is at least the number of digits. If so, return a 0 number immediately. (This is to prevent allocating a block of size zero.)
- Then, allocate a buffer of size `numDigits + places`. (This will be smaller than `numDigits` if places is negative.)
- Now, start a for loop from 0 to that sum:
    - Compute `i - places`.
    - If that is negative, write a `0` into the buffer.
    - Otherwise, write that digit from `a` into the buffer.
- Then make a `BigNumber` with the answer and delete the buffer.

OK, addendum from later on... I also need a function to return the lowest K signficiant digits of a number (so {65,1333,222}, 2 returns {65,1333}).
`getLowestNDigits(n)`:
- Actually, this is easy.
- Get the number of digits of this number.
- If it is smaller than `n`, return `this`.
- Otherwise, use a clever trick: call the byte-array constructor with our own digit array, but with `n` as the parameter. It will allocate a size-`n` array and copy the values!

#### Multiplication Hassle
Now that I have done all that, we can do multiplication of two `BigNumber`s.
But I realized that for large numbers this is slow. So I will turn to the Karatsuba algorithm whenever both numbers are over a certain number of digits.
If both numbers are big:
- Let N equal (smaller size)/2, rounded up.
- Obtain a1 (by shifting `a` by N), a0 (by taking the smallest N digits of `a`), b1, and b0.
- Let z2 = a1 * b1 and z0 = a0 * b0 (recursion!).
- To get z1, compute (a1 + a0) * (b1 + b0) - z2 - z0.
- This number should always be positive, so no error checking needed.
- Finally, the answer is (z2 shifted 2N) + (z1 shifted N) + z0.
Otherwise, use the following algorithm:
- Create a buffer of size `aDigits + bDigits + 1`.
- Create an `int` variable to store the column sum, starting at 0.
- For each number from 0 to `aDigits + bDigits`:
    - Create 2 counter variables `ca` and `cb`. Set one to `i` and the other to `0`. This represents multiplying `a.getDigit(i)` by `b.getDigit(0)`.
    - If `ca` is outside the range of a's digits, bring `cb` up to compensate and reduce `ca`.
    - As long as both indices are in range:
        - Multiply the corresponding digits (as `int`s) together.
        - Add them to the column sum.
        - Move `cb` up and `ca` down.
    - Finally, take the column sum mod BASE and store it into the buffer.
    - Carry by shifting the column sum right by 16 bits.
- At the end, if there is a carry left over, deposit it into the final slot.

#### Subtraction and Division
The next major goal is division. Before that I need subtraction and before that I need to be able to compare two numbers.
This calls for a new function:
`compare(BigNumber, BigNumber)`
- Will return `-1` if the first is smaller, `1` if the second is smaller, and `0` if both are equal.
- Get the number of digits of each.
- Use the `?:` trick to find the larger such number.
- Loop from the most significant digit downwards (i.e. start at N-1):
    - If A's digit is less than B's digit, return `-1`
    - If B's digit is less than A's digit, return `1`
    - Otherwise the loop keeps going.
- If we exit the loop without returning, all digits are equal; return `0`.

This allows me to immediately define the 4 compare operators (`> >= < <=`).

Armed with that, it is time for **subtraction**:
- It is probably easier to work in a buffer for this.
- First, check if `a` is smaller; if so, throw an error. (This is far better than simply returning 0, because it signals a programming error.)
- Otherwise, get the number of digits of a.
- Create a buffer of that size and copy the digits of `a` into it.
- Now, before we start the for loop I want to create an `int` variable called `borrow`.
- Start a for loop (forwards) over a's digits.
    - Convert the a and b digits to `int`s.
    - Compute `aDigit - bDigit - borrow`, and store it in a variable.
    - If that number is non-negative, store it directly into the buffer and set `borrow` to 0.
    - Otherwise, add `BASE` to it, store _that_ in the buffer, and set `borrow` to -1.

##### Division
Subtraction enables me to start thinking about division.
Just like how I defined a single-digit multiplication, I have a "single-digit" division, but here the idea is that the QUOTIENT is assumed to correctly be a single digit.
Actually, my final multiplication algorithm does not use the multiply single digit function but I am glad I kept it.

My first try at a single-digit division was too slow. Part of the problem is that my binary search that finds the divisor in 8 attempts is too inefficient. In reality, 

`singleDigitDivide(a, b)`:
- Get the leading digits of a and b and which digits they are.
- 
- Create 2 variables for our low and high guesses.
- Loop until the two bounds meet:
    - Our `guess` multiplier is the average of `low` and `high`.
    - Multiply `b` by the guess (converted to the digit type), and `compare` that to `a`.
    - If we get a 0, we found an exact match (!!). In that case, stop immediately.
    - If we get `a` bigger, then we have not guessed high enough (possibly). Bring the lower bound up to `guess`.
    - If we get `a` smaller, then we have guessed too high. Keep `quotient` as is.
- Return `quotient`.

Now, finally, it is time for the long division challenge!
- `deflate` the numbers so they have no leading 0s.
- Again, check if `a` is smaller than `b`; if so, return this time a zero `BigNumber`.
- In any other case, `deflate` the numbers first.
- Get the number of digits of each number.
- Create a buffer of size `aDigits - bDigits + 1`.
- Create a `BigNumber` to store the remains of `a`.
- Loop backwards over that buffer:
    - For entry `i`, shift `b` upward `i` digits, storing it in a new variable.
    - Find the single digit quotient for the shifted `b`.
    - Multiply that quotient by the shifted `b` and subtract it from `a`.
    - Also store that quotient in the array.
- Construct a `BigNumber` from the array!

With that done, the **modulo** operator is a "trivial one-liner":
- mod(a,b) is `a - (a/b)*b`

However, modular EXPONENTIATION is much harder!
- alwaysStartWithSanityChecks()
- First, we need to be able to convert `exp` into binary. But we can already get individual digits.
- Get the number of digits of `exp`, and multiply that by 8.
- Create 2 running total numbers: `product` and `multiplier`.
- Loop over the number of bits:
    - Bit `i` is obtained by getting digit `floor(i/16)` and finding the `(i % 16)`th significant bit. I can use some tricks to sleekify it.
    - If bit `i` is nonzero, multiply `product` by `multiplier` (which is our current power of 2) and take the result mod `modulus`.
    - Also, no matter what, square `multiplier` modulo `modulus`.
- When done, return `product`.

##### Converting to Hexadecimal
I have no idea why this needs a level 5 heading, but I'll make one just because.

Anyway:
- Allocate a `char` array 4 times "bigger" than the digit array (because each digit is 4 hex chars).
- For each digit:
    - Create a variable to store the digit's value.
    - Loop 4 times:
        - Get the least significant 4 bits of the value.
        - Put it in basically the reverse position on the `char` array.
        - That is, size - 1, minus 1 for each time thru the loop, minus 4 for each digit we have considered.
        - Um, because these are individual numbers from 0 to 15, we just use an if statement to map them directly onto characters.
- When done, construct a `std::string` containing the char array, then delete it. (I checked, `std::string` does a deep copy.)

### Other Odds and Ends for RSA Encryption
At this point, I still do not have the means to generate RSA keys because I cannot immediately tell if a given number is prime or not.

First, I can kill all but about 7% of candidate primes with this test:
- If the first digit is even, return `false`
- Otherwise, loop over a (pre-computed; this explains the Python file floating in the directory) list of primes. If the number divides any given prime, stop and return `false`.
- If no prime factors are found up to 997 (or whatever), return `true`.

The Fermat test just computes a^n mod n, and checks if the result equals a.

The Miller-Rabin test works as follows:
1. Determine how many times we can take `n-1` and divide it by 2: (If the number was even originally, return `false` immediately.)
    - To do this, get the first "digit".
    - If that "digit" is even, immediately return `false`.
    - If that "digit" is `1`, then we have at least 32 (!) shifts available. In that case, I would rather just return `false` and force them to throw me another prime candidate.
    - Otherwise, get its value and subtract 1. This will give an even number.
    - Create a counter variable.
    - While the number is even, divide it by 2 and increase the counter.
2. Compute 2^(counter). Divide the big number by this, and call that `d` or something.
3. Create a for loop to run for a certain number of times; I think 5.
4. Call `rand()` to get a "random" number. (I will need a more secure random generator later but for this function `rand()` will suffice.) Create a `BigNumber` out of that. (Yes, these values will probably be in the range 0 to 32767.)
5. Let `x` equal a modular exponentiation of `a^d mod n`.
6. If `x` equals `1` or `n-1` (which I may as well have computed at the beginning), the number is more likely to be prime; loop again.
7. Otherwise, repeatedly square `x` mod `n`, and if it ever reaches `n-1` then loop again.
8. If it does not, the number is absolutely composite!!
9. Return to step 4 until we are at the required number of loops.
10. If no absolute composite witnesses are found, the jury concludes that the number is probably ~~not guilty~~ prime.

With that, I can... **Oh Wait**.

### Detour: SHA-256 and SHA-512
Yeah, I knew this was coming. At least I can get away from a currently 1142-line file for a while.

I'm getting the algorithms for these from Wikipedia so not much will be written down here.

OK, something is a little vague: "[C]opy chunk into first 16 words w[0..15] of the message schedule array". 
- Loop over the 16 slots of the MSA.
    - In that slot, we want to put bytes A, B, C, and D in that order in there.
    - Make an unsigned 32-bit int variable.
    - Create a second for loop from 0 to 3:
        - Shift what was previously in the int 8 bits left.
        - Then add the new content.
    - When done, insert the int into the MSA.

Also, my function returns a string (of hexadecimal). So, after I get the final hash value (in `h0, h1, ..., h7`):

I also found it easier to use this method to hash a string (not a byte array with a length):
- Get the string to hash, and also its length.
- Create a buffer of that size.
- Loop over the string:
    - Copy the value of the character into the buffer.
- Finally, pass that buffer and the length into the sha256 function.
- Delete the buffer, then return the hash.

## Messages

After some disappointing results with the SHA and BigNumber classes let me get going on the actual structures that make this project possible.

I cannot really justify abstraction here. The public message contains nothing the secret message does not also somehow contain, albeit in a somewhat different form.
And upon further reflection, the secret message is rather unremarkable too because it is the RSA keys that know how to en/decrypt messages.
So I will consolidate them into one class with an additional boolean for is the message encrypted or not.
Because of the potential for encryption, I will call the possibly encrypted text the "stored" subject/body, and the decrypted text the "plain". This means I have 4 total getters for just these.

For retrieving the "plain" text:
- If `encrypted` and `decrypted`, return the recorded plain text.
- If `encrypted` but not `decrypted`, return a string like `[Encrypted]`.
- If not `encrypted`, return the stored text.

I also have a `recordDecryption` that allows you to record the decrypted message. This way you only have to do so once per session.
It just assigns the two strings passed to the "plain" parameter and turns on the `decrypted` flag.

## Linked List
I dont believe in Andy's iteration functions that simply print the associated values.

As usual for my linked lists I only give them the minimum amount of functionality in the class to allow my other code to use them.

Of course, everything in this is being done with pointers which means I have to be very careful with deleting.

...and, of course, I find out that I cannot do the usual .h/.cpp thing. I guess... um...

### Node
...has a payload pointer of type `NodeType` and a pointer to the next and previous nodes and some getters. Again, not very exciting.

### LinkedList
...has two pointers (head and tail) to Nodes.

The destructor is not trivial. Since I chose not to use chain-deletion I have to loop to delete everything manually.
- Declare a `currentNode` pointer
- While it is not null:
    - Get its `next`
    - Delete it
    - Move to the previously-gotten `next`

The usual stuff abound with getters to the head and tail.
We need to be able to add and remove nodes.
I also have getters to get the nth element (from either direction) and the total number of links.

Implementations:
Constructor: Only null constructor. Set head and tail to 0.
Destructor: ...as above
`getHead(), getTail()`: trivial
`insertHead(Node*)`:
- Set that node's next to the old head
- Set the old head's prev to that node
- Make that node the new head
`insertTail` is symmetric.
`remove(Node<T>*)`:
- If the node equals the head:
    - Get this node's next.
    - Set the new head to that node (or that null).
    - If the next is not null:
        - Set the next's previous to null.
- Otherwise:
    - Let A be its prev and B its next.
    - Set A's next to B.
    - Note that we DONT set B's prev here because we do that in the reverse check.
- If the node equals the tail do the mirror of the above.
- Finally, delete this node.
`getFromHead(int)`:
- Make a pointer (to head).
- Loop up to the int:
    - If the pointer is non-null, set it to its next.
- Return the pointer.
`getFromTail(int)` is symmetric
`getSize()`:
- Make a variable `count`
- Start at head.
- While non-null:
    - Advance to the next node
    - Increase the counter
- Return the counter

## User

Well, looking at my UML, it would stand to reason that the next thing to work on is the user system.

Authentication and encryption is going to be a challenge, so I will focus on the messaging system first.

Each user has a linked list of messages. They also will have public and private keys and credentials but those can wait.
Actually I should get the `Credentials` made so that I can store usernames.

### Credentials Detour
So my credentials will have a username, a salt, and a hashed password string.

The constructor takes a username, a salt, and a password. I then compute sha256(username + salt + password). I know this is outdated but come on, I had to write the sha256 by hand!!

Each of the three fields (username, salt, hashed password) will have getters (because I need to store them in the database).

`checkLogin` computes the sha256 using the formula above. Then it checks it against the stored value and returns whether or not it was a match.

`changePassword` takes the old and new passwords.
- Call `checkLogin()` on the correct username and the old password.
- If it matches:
    - Hash `username + salt + newPassword`
    - Store that in the hashed password.
    - Return `true` for success.
- Otherwise return `false` for failure.

### Back to User

I am going to use **encapsulation** here: the linked list is never directly accessible given the User object. (Of course, with the insecurity of C++ you could probably obtain it by abusing pointers.)

Null constructor:
- Make an empty credentials and an empty LL of messages.

Constructor:
- Right now, we are given username, salt, and password.
- Create a `Credentials` with the username, salt, and password.
- Create an empty linked list of messages.

Methods:
`showMessages()`:
- Basically, show a summary list consisting of, for each message, who sent it, when, and the subject (or the string `[ENCRYPTED]` if it is).
- Create an int to keep track of which message is which.
- If the node is null, say you have no messages. Otherwise:
- Loop over messages.
    - Print the following info: counter, `U` if unread, date, sender, subject (if known).
    - Add `1` to the message number.
- Nothing left to do outside the loop. I guess print something like "End of messages".

`checkLogin(string, string)`:
- Just a pipeline to the `credentials` method of the same name.

`addMessage(Message*, bool)`:
- If `bool`, add the message to the tail, otherwise add to the head.
- Um... that's it. Wait, no, first we have to make a `Node`, but that is too easy.

### RSA Key

So the next step on my list is to implement public and private keys.

Before I can move on, I need to go back to the BigNumber file and write a method to generate large primes of a certain length.

And before I can do that, I need a way to generate random numbers.
The only thing I can really do is hash user-given gobbledygook (i.e. have the user randomly type at least 100 keys) and also throw in things like the time and maybe even the username or something.

That sounds like another class.

#### Randomizer
The randomizer data will consist of a string and a stringstream to hold the hex digits awaiting release.
I'm starting to realize how many obvious security concessions I am making to get this made as a C++ project in 2 weeks.

Before I even do the constructor I need a function to get the time, because that happens a lot. Mostly this is just a bunch of annoying c++ functions.

Null constructor:
- Make a string stream.
- Put the current time, in nanoseconds, into that stream.
- Set the `cursor` to zero, so it knows to generate more bytes. (This is so one can call the `giveExtraStuff` method to add more randomness to the buffer.)

The constructor taking a string:
- Make a string stream.
- Put the current time in nanoseconds into the stream, followed by a semicolon and then the passed string.
- Then it calls the `getMoreBytes` method to fill the buffer.

There is a method to append extra data to the randomness string.

There are four other methods (one is protected).
`giveExtraStuff` takes a string and appends it to the seed.
`getMoreBytes()` fills up the byte stack:
- Hash the seed data (and append the current time to the end). The length will always be 64.
- Loop 32 times:
    - Make an int variable.
    - Put a `1` and the next 2 characters into the stream.
    - Read out a hexadecimal int.
    - Subtract 256.
    - Put that into the class's byte array.
- Finally, set the cursor to `31`.
`randomHex(int)` returns `int` hex digits by reading from the stack and calling `getMoreBytes()` if needed.
- Make a stringstream.
- Loop from 1 to the number of digits:
    - Try to read 1 character.
    - Check if the queue ran out (EOF bit set).
        - If so, the character read is junk.
        - Ask for more bytes, and read the character again.
- Return the string of hex digits.
`compact()` just sets the seed data to its own hash. This is useful if it has grown to really long lengths.

#### Back to BigNumber
With that, we can close off the BigNumber class with a `generateLikelyPrime` method and a `gcd` function.

The `gcd` follows the standard Euclidean algorithm. We store the higher number in `higher` and the lower number in `lower`.
Then we repeatedly do `(h, l) -> (l, h % l)` until the latter is zero; then `l` is the gcd.

The `generateLikelyPrime` will:
- Create a `BigNumber` object to store the result.
- Make a keep-going loop:
    - Get the number of hex digits as `(bits + 2) / 4`.
    - Use the generator to get the appropriate number of random digits.
    - Append a "1" to the end of the string to ensure the number is not even.
    - Construct a number using those hex digits.
    - Check if it `isProbablyPrime`.
    - If so, set `keepGoing` false and return it.
    - Otherwise, reject the number and loop again.
- Upon exit, return the generated probably-prime.

This process is slow, but not unbearable. It should finish within about 10 seconds.
When testing I could perhaps turn the prime size down to 250 bits which in theory should take 1/8 as long (mod exp seems to be O(bits^3)) or less (because primes are more common).

Now that we have _that_ out of the way, it is time for:

#### Back to RSAKey
Finally, we can have a method to create key pairs!!

We will make a `setup` method. The null constructor will instantiate the key values to low numbers like N=899 (29x31) for the sole purpose of not wasting time.

The constructor given an `int` and a `Randomizer` will call the `generate` method below.
The constructor given the three key parts will just directly assign the fields.

The `generate` method will take a number of bits for the key size and a random generator. It will:
- Generate two big primes `p,q` of approximately half the number of bits. Actually, I think giving `p` four fewer bits and `q` four more bits will work better.
- Multiply them to get `N`.
- Also, multiply `(p-1)` by `(q-1)` to get the phi of N.
- Have a list of preferred primes at the ready. (These are primes between 2^16 and 2^17 with at most 4 non-zero bits. Almost every value of N will support 65537 or 65539.)
- Set a bool `keepGoing` to true.
- Loop over those primes as long as we `keepGoing`:
    - Let `e` be the current prime. (Almost all of the time the first one will suffice.)
    - Use a brute-force method to get the decryption key `d`.
    - The formula is `d = (k * phi(N) + 1) / e`, where `k` is chosen to make that an integer.
    - Loop from `k=1` to `n-1`, also with `keepGoing` in the loop condition:
        - Compute `k * phi(N) + 1`.
        - See if that number mod `e` is 0.
        - If so, we are done. Stop searching, and set `d` equal to that number modulo `N`.
    - If none work, then we just try the next value of e.
- After we have found the D, assign all fields. It is ready!

`encrypt` and `decrypt` are very similar on the inside, but there is some pre-processing that needs to be done before a message can be encrypted.

To encrypt:
- We are given a `std::string` of the plain-text message the user is sending.
- First, reject any message over 500 characters.
- Next, create a `stringstream`. This will hold the encrypted chunks as each one comes out.
- Next, loop over the length of the message in increments of 100:
    - Get the 100-character substring starting at the loop index.
    - Create a `stringstream`. Set it to 2-character hex mode.
    - Loop over each character of the chunk...
        - Convert the character to an int and then put the int in the stringstream.
    - When done, we should have the string converted to hex.
    - Now, create a `Randomizer` with the message as the seed. (This is to ensure that the same message encrypted twice does not come out the same.)
    - Get 32 digits from the randomizer and throw them in the stream.
    - Finally, get the `str()` of the stream.
    - Construct a `BigNumber` from that hex.
    - Encrypt it using the formula `m^e mod N`.
    - Convert that into hex, and toss it down the outer stream (of encrypted chunks). Put a comma in as a separator.
    - And we're ready to loop again!
- Finally, return the extracted string from the encryption stream.

Decryption is going to be interesting. The data comes in chunks separated by commas. I basically have to undo everything I did in the encrypt method... quite carefully...
- Make a string to store the message. (I dont trust stringstreams for messages.)
- Put the encrypted text in a stringstream.
- Create a while loop, with a `getline` in the condition.
    - If the string is not empty:
        - Convert it to a BigNumber.
        - Decrypt the BigNumber using `cipher^D mod N`.
        - Shift the result right by 4 BigNumber-digits (each digit is 32 bits, so shift by 128 bits) to remove the hash.
        - Convert the result into hex.
        - Now, do the all-too-familiar loop to turn that hex into actual text. Create a stringstream and loop:
            - Create a what, third?, stringstream inside the loop.
            - Do the 1 trick, tossing 2 characters in.
            - Read them out as hex, and subtract 256.
            - Convert that into a char. Toss that char into the stringstream outside the loop.
        - After that is done, we have part of the message! Append it to the message string.
- Return the concatenated message string. We're done!

### Back to User (again)
So now that I finished the RSA key system, I can add it to my Users. This will of course change the constructor and some other stuff.

There are 2 non-null constructors.
`(std::string, std::string, RSAKey, Randomizer)` is used to create a new user account. The first two parameters are username and password.
`(Credentials, RSAKey, std::string)` is used to read in an existing account. The string is the masked version of the decryption key that the user has not yet unmasked.

The other security operations we need are:
`maskKey(password, generator)`:
- Check that a key does indeed exist; if not, throw an error.
- Convert the decryption key into hexadecimal.
- Generate a random salt using the generator. Store it in the user object; we need to remember this!
- Create a stringstream.
- Start a for loop over the length of `dHex`, but jumping in increments of 64, the length of a SHA256 hash:
    - For this chunk, we hash the password plus the salt plus the string representation of the loop index. (This is just so the chunks are all different.)
    - Loop from the start of the chunk to the end (or the end of the dHex string):
        - Both `dHex` and the hash have a hex character.
        - Convert those into ints, add them, and then put that into the stringstream.
- Return the encrypted string.

Similarly:
`unmaskKey(password)`: There are two stages. The first involves decrypting the masked key. The second involves checking that the key actually works; if so, we return true.
- Proceed similarly to the mask function. We already have the digits of the masked key and the mask salt, and the password.
- Make a stringstream.
- Outer loop in 64-jumps as before:
    - Use exactly the same hash system (password + maskSalt + chunk index) to get the hash for decrypting.
    - Loop from 0 to 63:
        - If the index is in range:
            - Convert both corresponding characters to ints.
            - Now, compute (16 + cipherdigit - hashdigit) mod 16 to remove the effect of the hash digit.
            - Put that digit into the stream.
- We now have a hex stringstream. Construct a big number.
- The next step is to test this potential key.
- We compute (42^e mod N) = x, and then x^d mod N.
- If the answer is 42, the key is successful!
- Otherwise, they fail. 

#### More Message Methods
I will also implement `getNumMessages()` and `getMessage(int)`. This sort of functions like an array get but you cannot change the internal structure of the linked list.
For `getMessage` I am not quite sure if I want to return a pointer or a reference to the Message object.

There is also `deleteMessage(int)`. This just loops to find the appropriate node, and then calls `remove()`.

There is also `readMessage(Message*)`. It looks trivial but really is not because you have to be able to decrypt ciphered messages.
So let's begin:
- First, if the user is not "logged in" (i.e. has the D key), return false. (I notice a lot of my methods are growing boolean returns...)
    - Actually, never mind. Just display a message to the user and don't return a false.
- Now, create a string to hold the message subject, and another for the body.
- If the message is encrypted:
    - If it is already decrypted, set the string to its already-computed plain text.
    - Otherwise, use our key's `decrypt` method on each piece.
- Otherwise, just get the plain text and store it in the strings.
- Use `cout` to present the user with something like:
```text
From: [username]
Sent: [date mm/dd/yy hh:mm]
Subject: [subject]
Body...
```

## FileSystem
So I decided to split this off and potentially make 2 systems (probably not the second tho). It will be much easier to make a system that just writes everything to a file when you are done, because I don't at this moment want to mess with SQL.

The constructor doesn't do anything at the moment other than set everything to empty defaults.

So the file system needs an `init()` method. It will take a filename and create a new `ifstream` to read the file.

My format will be: (`|` is literal, `< >` are not)
```text
BEGIN USER: <username>
<N>|<E>|<maskedD>|<maskSalt>
Begin Message
>>> <ID>|<encrypted=1,not=0>|<date>|<from>
>>> <subject>
>>> <eachLineOfTheBody>
>>> <moreLinesMaybe>
End Message
Begin Message
[...]
End Message
END USER
```
Like many things, I keep thinking about how anyone with access to the file could easily tamper with this. I don't know of a good way to avoid that.
The `>>>`s on message body lines are so that someone cannot have a message that says, e.g. `End Message\nEND USER\nBEGIN USER:` (format injection).

Anyway, the `init()` method will try to find the file with the specified name. If it does not exist, the system will start up empty. Otherwise, it will read line by line trying to match the format above.
OK, that's not specific enough.
We need a User pointer to store which User we are currently building (or null if we are not) and some strings to hold the components of the messages.
Oh, and while we are at it, store the file name in a string so that we remember where to write the data to.
So in a while loop until the file runs out:
- Get a line.
- If it starts with "BEGIN USER", then the username is everything at position 12 and after.
    - Get that username.
    - Then read another line to get the key data.
    - Put that key data into a stringstream.
    - Use `getline` with a `|` to parse the N, E, masked D, and mask salt.
    - Convert the N and E into `BigNumber`s.
    - Construct an RSAKey object using the N and E.
    - Construct a `User` on the heap.
    - Create a second while keep-going loop:
        - Read a line.
        - If it says `Begin Message`:
            - Read 2 more lines.
            - Put the first into a stringstream.
            - `getline` up to the first space, and discard that string.
            - Then `getline` to `|`. This string is the message ID.
            - Then `getline` to `|` again. Read that as an `int`. If that int is 1, the message is encrypted.
            - Finally, `getline` to get the date.
            - The second line contains the subject; just get the substring starting at the 4th character.
            - Finally, we must get one or more lines of the body.
            - Create a stringstream for the body content.
            - Create a third (!) keep-going loop:
                - Get a line.
                - If it starts with `>>>`, toss it (minus the `>>> `) in the stream.
                - Otherwise assume it is "End Message" and quit the loop.
            - At this point we will have hit the end of the message.
            - Create a new `Message` object on the heap and add it to the end of the `User`'s message list.
        - If not, it may say `END USER`?
            - If so, exit this loop (the middle one).
        - If not:
            - Did we hit end-of-file? If so, exit all loops.
            - Otherwise, something is seriously wrong.
    - At some point we hit an `END USER` or the end of the file.
    - Append this user to the end of our linked list of users.
    - If EOF, set the outer keep going to false.
- If, in the wasteland between two users, we find anything other than `BEGIN USER: `, ignore it. But perhaps for debug purposes we might want to record them.

That will successfully initialize the user list.

I think that is all for the initialization phase. But we also need the reverse process, i.e. saving the info to a file.

This is actually easier because I don't have to do (or avoid) validation.
- Create an output file stream and open the file.
- First, loop over all users.
    - Write `BEGIN USER: ` followed by the username.
    - Then get their N, E, masked D, and mask salt.
    - Write those, in that order, separated by `|`s.
    - Loop over the user's messages:
        - Write `Begin Message`.
        - Get the message fields: ID, sender, date, is encrypted, stored subject, and stored body.
        - Simply put the ID, encrypted flag (as a number), unread flag (as 'U' or 'R'), date, and sender's username into the string, with `>>>` before and the fields separated by pipes.
        - Then put another `>>>` followed by the subject.
        - Then we need to do something tricky for the body.
        - Make a stringstream.
        - Put the body into the stream.
        - Also make a string `line`.
        - Keep-going loop:
            - Get a line from the stream.
            - Put `>>> ` (with a space), then the line, then an endl into the file.
            - If we hit EndOfFile, set keep going to false.
        - Write the words `End Message`.
    - When done with messages, write `END USER`.
- Finally, close the file!

The next system method I need is the login, given a username and a password will return a user pointer or null.
- Create a User pointer (set to null).
- LinkedListLoop over users:
    - Call the current user's login method.
    - If it is true, return that user pointer.
- If nothing was already returned, return the null pointer.

At this point I think I will start working on the menu and then see if I need any fill-in methods as I go.

#### Methods I Realized I Would Need
`isUsernameValid(std::string)`:
- Get the length of the string.
- If it is less than 2 or greater than 15 return `false`.
- Loop over each character:
    - If the character is a space or newline, return `false`.
- Return `true`.

Originally this was called `isUsernameTaken` but I wanted `true` to mean you COULD use that username, so:
`isUsernameFree(std::string)`:
- Loop over all users.
    - Get the username of this user.
    - If it matches the std::string exactly, return `false`.
- Return `true`.

`isValidPassword(std::string)`:
- Get the length.
- If under 8 or over 20, invalid.
- 3 booleans: hasLetter, hasDigit, hasSpecial.
- Loop over all characters in the string:
    - If the character `isalpha`, set `hasLetter` true.
    - Otherwise, if it `isdigit`, set `hasDigit` true.
    - Otherwise, set `hasSpecial` true.
- If all 3 booleans are true, return true.


#### Main Function

The code will go in a static method called `main`. Yes, I could not resist borrowing one thing from Java.
The overall flow seems to be this:
1. Load the database
2. User can log in or create an account
3. User interacts with their account
4. User exits, data is saved to the file.

Because I am going to be getting a LOT of numbers from the user, I will make a function.
`getNumberFromUser()`:
- Create a string and call `getline`.
- Try to call `std::stoi` on the number.
- If it works, return the number.
- If any exceptions are thrown, return -1.

#### Step 1: Initialization
- Create a `FileSystem` object.
- Call `init` with the file "userdata.txt".
- Create a `Randomizer` object, to get random numbers.

#### Step 2: Login/CreateAccount
- Create a User pointer.
- Start a keep-going loop:
    - Give the user a menu to either log in or create a new account.
    - Get their choice as an int.
    - If they choose 1 (log in):
        - Ask them to enter the username.
        - Ask them to enter the password.
        - Call `login` on those credentials. Store the result in the `you` pointer.
        - If it returned a user, set `keepGoing` to false.
        - Otherwise, display "Incorrect username or password" and loop again.
    - If they choose 2 (create account):
        - Ask them for a username.
        - If the name is invalid, say so.
        - Otherwise, if the name is not free, say so.
        - Otherwise:
            - Ask them for a password. Warn them NOT to use their actual IU password or any other password they already use.
            - Ask them to repeat the password.
            - If the passwords match:
                - Ask them to type at least 100 random characters.
                - Create a stringstream.
                - KeepGoingLoop:
                    - Get a line from the user.
                    - Put it in the stringstream.
                    - Feed the current time into the randomizer using its own method.
                    - Check the length of the stringstream. If at least 100, stop going.
                - Feed the contents of the stream into the randomizer.
                - Ask the user to press enter to continue (just in case they accidentally hit the enter key).
                - Use the randomizer to construct an RSAKey.
                - Use the randomizer, the username, the password, and the key to construct a User on the heap.
                - Add this user to the list.
                - Set the outer keep going to false.
                - Welcome the user.
            - Otherwise (the passwords do not match):
                - Tell the user passwords do not match and cancel the operation.
    - If they choose 0, return. (There is no need to write to the database since nothing could have changed.)
- At this point, we have to have a valid user either newly created or gotten from the existing list.

#### Step 3: User Interaction
- The main menu consists of the following:
```text
(1) View your messages
(2) Send a message
(3) Change your password
(0) Log out
```
- Present the user with this menu.
- Ask them to pick a choice.
- If they pick "1", send them to the View Messages loop which I will write a separate function for.
- If they pick "2", send them to the Send Message loop which I will also write a separate function for.
- If they pick "3", send them to the Change Password method.
- Otherwise, set `keepGoing` to false.

OK, that just kicks the can down the road. I will write algorithms for the 3 methods I skipped over below.

#### Step 4: Close Down and Write Data
- Wish the user good bye and give a hand-washing warning.
- Write everything to the file.
- And I **think** that is all we need!

#### About those pesky functions...
`viewMessages()`, you see, does not just list all your messages. It enters an entirely new loop that allows you to read and reply to messages (i.e. it can even invoke `sendMessage`).
Actually, that raises an interesting point. Both "send a new message" and "reply to existing message" use the same basic code.
So I should make a function that does all the work given a recipient and subject.

Let me begin with `viewMessages(User*)`:
- Create a keep-going loop:
    - Show them the table of their messages.
    - Ask them to enter the number corresponding to which message they want to read or "0" to quit.
    - If they enter a number N:
        - Get a pointer to message number N.
        - Read that message.
        - Give them a "press enter to continue".
        - Create a KG loop:
            - Ask them to choose to either go back, read the message again, delete the message, mark it as unread and exit, or reply. (note: reply does not include original message text)
            - If they choose to delete it:
                - Ask them to confirm deletion by entering `y` or `yes` (or anything starting with `y` or `Y`).
                - If so, delete message, and don't keep going.
            - If they choose to mark it as `unread`:
                - Mark the message as unread. Exit the loop.
            - If they choose to read the message again, just call `showMessage` again. Don't exit the loop.
            - If they choose to reply, then open the method that deals with replies. When finished, exit this loop.
            - If they choose to go back, exit the loop.
            - Anything else results in a message. (This and other failures, such as delete and then no confirm, may cover up the message itself; the "re-read" feature is there for a reason!)
        - When that finishes, the outer loop runs again.
    - If they enter 0 (go back): Exit the loop.
    - If they enter anything else, tell them it was an invalid choice and they should be ashamed of themselves. (Well, maybe not that last part.)

`composeMessageMenu` given `userPtr` only
- First, display a list of all the users currently in the system with numbers attached.
- Have the user pick a number.
- If that number corresponds to a user:
    - Ask the user to enter a subject.
    - Then, go to the other `composeMessageMenu` function to handle the rest of the process.
- Otherwise, do nothing, let the user know it was invalid, and cancel.

`composeMessageMenu` given `userPtr, toWhom, subject`:
- The last piece we need to get, specifically from the user, is the message body. Which is also the hardest!!
- Note: I will limit messages to at most 10 lines (and also 500 characters).
- Tell the user that they can include at most 10 lines and they have to do them one at a time.
- Make a linked list of strings.
- Create a keep-going loop. (This resembles the message viewer...)
    - Display the current contents of your message. (This is done by looping and using some cout's. I've done so many of these I know what I am doing.) Also in the loop count characters.
    - Ask the user to either change a line, add a new line, insert a new line, delete a line, send the message, or cancel the message.
    - If they choose to change a line:
        - Ask them which line (0 to cancel).
        - Ask them to enter its new contents or enter nothing to cancel. (This is a lie but it works.)
        - Present the contents of the new line and ask for one last confirmation. If so, change the line.
    - If they choose to add a line:
        - If there are already less than MAX_LINES:
            - Ask them for the contents
            - Add it to the list
        - Otherwise, say the have maxed out and suggest deleting a line or changing one of the existing ones.
    - If they choose to insert a new line:
        - If there are less than MAX_LINES lines:
            - Ask them to enter their line
            - Then ask them where to insert it. 0 = cancel, 1-N = before that line, N+1 = add to end
            - There's ~~an app~~ a method for that!
        - Otherwise, give a similar error message to the case above.
    - If they choose to delete a line:
        - Ask them which one. 0 = cancel.
        - If they enter a valid number, get the node and then delete it.
        - Otherwise give an error message.
    - If they choose to send the message:
        - Get the date+time in the format mm/dd/yy hh:mm. (C++ has a ~~plan~~ function for that!)
        - Stringstream
        - Loop
            - Put line into stream followed by endl
        - Push ~~button~~ method get ~~mortgage~~ string
        - Remove trailing newline using pop_back()
        - Uhh... I think sending is actually non-trivial.
        - We have the other user.
        - Get their public key.
        - Use the public key to encrypt the subject and body.
        - Construct a Message (on the heap as always) with the:
            - ID (just be 0, this was more for SQL anyway)
            - Sender (`userPtr->getUsername()`)
            - Date (check)
            - Subject (check)
            - Body (check)
            - Unread (always true)
            - Encrypted (true)
        - Call the other user's `addMessage` routine!
        - Woohoo!
        - Put "Message Sent!" or something.
        - And definitely don't keep going!
    - If they choose to cancel the message:
        - Are you sure you want to discard this message?
        - If so, just exit and do nothing else.
    - Otherwise, display an error message.
    - Press enter to continue...
- I think that's all!

`changePasswordMenu`:
- OK, this will be way easier.
- Ask the user to re-enter their password.
- Do a check login. If it passes:
    - Ask the user for their new password.
    - Check validity. If it passes:
        - Ask to confirm password.
        - If they match:
            - I believe the User's `changePassword` function takes the old and new passwords and if true changes the password.
        - Otherwise, tell the user they failed and should feel bad. (Maybe not the last part there.)
    - Otherwise tell them the password is invalid.
- Otherwise tell them the password is wrong.

## ~~SQLSystem~~

I'm not going to try to make the SQL system work because I only have three days left and I want to get something that I can demonstrate.

> With that, I think I have all the pieces together to start building the system. In fact, now would be a good time to back up to GitHub.
> 
> OK, I'm back. 
> 
> The SQLSystem is really just a class designed to communicate with the database. As such, I do not even really store a LinkedList of Users or anything.
> 
> It needs a SQLite3 database pointer object. Not entirely sure what else, but if I need it, I can put it in.
> 
> Maintaining the user objects is tricky. I think the best way to deal with this is to have the linked list contain "shells" of users (with just the credential data).
> Then when the user logs in, they receive a copy, and that copy contains the message data and their actual decryption key.
> 
> 
> I guess before I implement methods, I should have a clear idea of the database.
> I envision many tables. There would be one table for users, which would list basic information about each user: username, public key N, public key E, masked private key D, and the salt used in the masking algorithm.
> Then, each user would have a table that consists of the messages they have received. Each row would have all the characteristics of a message: id, sender, date, subject, body, is unread, is encrypted.
> 
> Methods:
> `init(string)`: Initializes the system. String is database filename.
> - Call `sqlite3_open` to open a database connection to the given filename.
> - 
> 
> `readUsers()`: Reads in a linked list of users. Ditches the old list, if any.
> 
> 
> `userLogin(string, string)`: Loops thru all users. 

