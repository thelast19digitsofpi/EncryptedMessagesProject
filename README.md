# Secret Messaging System

This was a thing I made a while back for one of my early computer science classes. It was basically a "make whatever you want in about three weeks" assignment; the instructor has confirmed that we are allowed to make these public.

I deliberately tried to make it as hard on myself as possible by using C++ when we were allowed to use Java as well.

**This project has no place in any kind of actual serious encryption**. Everything was designed for the sole purpose of working for its own purpose within the time limit. (That seems to be a weakness of mine, although I am getting better now.)
You will notice the original README states that I was "thinking about security". That does not mean that I had any legitimate hope to _make it_ secure; rather, I was _aware_ of how _insecure_ many of my components were (while trying to minimize the problems when feasible).

I haven't changed the inside of the project since when I actually wrote it (that would be April 2020 if I remember correctly -- it was right at the beginning of the COVID-19 pandemic) so there are several inconsistencies and whatnot.

The instructor for this class taught us to write "algorithm" files before writing the code itself. I probably should have done that in the .cpp files themselves instead of making one huge file, but "it worked for me", so...

This code is not licensed because it's not meant to be used. It had three purposes, and three purposes only: (1) to pass an assignment, (2) for me to learn, and (3) ...well, I'm not sure how much someone else would learn from it since I wasn't writing it with that in mind.

Anyway:

**THIS IS NOT SECURE.**

----

## To Run the Project
1. `cd` to the `base` directory
2. `make` builds it
3. `make run` runs it.
4. `make clean` removes the .o files and the app.
5. `make valgrind` and `make debug` work.

## Existing accounts
As it stands the database has 3 accounts.
- Username `alice`, password `abcd1234!@#$`
- Username `bob`, passwword `pi=3.14159...`
- Username `carol`, password `valgrind=sl0w!`
Passwords must be 8-20 characters and have at least 1 letter, 1 number, and 1 punctuation mark.

## Overview for Average Users
- The system lets you send secret messages to other users.
- To start, run the program.
- At the login screen, choose the "Create Account", or "Log In" if you created an account before.
- After logging in or creating your account, you have the main menu screen.
- You can send messages or read your existing messages. Note that all messages are encrypted, and decoding them takes a few seconds sometimes.
- You can also send messages. The process is a bit tricky to learn at first.
    - First, you are asked for who you want to send it to. You have to enter it as a number, **NOT** the username.
    - Then, you write the subject.
    - Then, you go to the Message Builder. Because of the nature of console editing, the message builder is a bit convoluted.
    - To add a new line, choose option 2 from the message builder screen. Then type your line and press enter.
    - To add another line, you must choose option 2 again.
    - You can also change the contents of a line and delete lines.
    - When you are ready, you must choose option 5, Send. Then everything is taken care of.
- To exit, usually you can just enter "0" several times until you reach the main menu, and then one final "0" will exit the program. DO NOT use Control+C or other methods as these will not save the data!

## Overview for Computer Nerds
This project is designed to allow you to send encrypted messages to others. Because I used C++ and did not notice the openssl library until rather late, I had to do it all on my own. (This was by choice. Most of the effort I put in to this would have been next to trivial with Java because the language has built-in security features.)

I was thinking a lot about security as I was building the project. Because I didn't have a lot of time, there were plenty of things that are definitely NOT secure, including but not limited to:
- `std::string`, which leaves fragments of the passwords all over memory
- The console, which has your passwords in plain text
- My password storage system (explained in more detail below). It is a bit more sophisticated than a salted hash, but it could probably be broken rather easily.
- The randomizer. Because again I did not know I had a secure random generator, I tried to build my own. It uses a bunch of hashes and times, but since most of the entropy is provided by the user, and that is in plain text on the console window, it is probably really easy to break.

The meat of the project is an RSA encryption system. I built an entire BigNumber class, all the way up to modular exponentiation (i.e. given `a, b, c` compute `(a^b) mod c`).

I made 
