fl = open("primelist.txt", "w");
num_primes = 0
s = ""
def sieve(n):
    array = [True for i in range(n+1)]
    p = 2
    while p*p <= n:
        for i in range(p*p, n+1, p):
            array[i] = False

        p += 1
        while not array[p]:
            p += 1

    return [m for m in range(2, n+1) if array[m]];

def prob(l):
    x = 1.0
    for n in l:
        x *= (1 - 1/n)
    return x

prime_list = sieve(65538)
i = 0
fl.write(str(len(prime_list)) + "\n")
for p in prime_list:
    i += 1
    fl.write(str(p))
    fl.write(", ")
    if i % 10 == 0:
        fl.write("\n")

fl.close();

print("Will leave about {:.2f} percent".format(prob(prime_list) * 100))

