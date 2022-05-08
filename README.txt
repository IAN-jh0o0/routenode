routenode
Spring22 Computer Networks PA2

This program implements each router in link layer

Explanation
1. To compile
$ make

2. Distance-Vector Routing Algorithm - Simple test cases
2-1. Regular mode without Link Cost Change
$ ./routenode dv r 1 1111 2222 1 3333 50
$ ./routenode dv r 1 2222 1111 1 3333 2 4444 8
$ ./routenode dv r 1 3333 1111 50 2222 2 4444 5
$ ./routenode dv r 1 4444 2222 8 3333 5 last

2-2. Regular mode with Link Cost Change
$ ./routenode dv r 1 1111 2222 1 3333 50
$ ./routenode dv r 1 2222 1111 1 3333 2 4444 8
$ ./routenode dv r 1 3333 1111 50 2222 2 4444 5
$ ./routenode dv r 1 4444 2222 8 3333 5 last 100

2-3. Poisoned Reverse mode without Link Cost Change
$ ./routenode dv p 1 1111 2222 1 3333 50
$ ./routenode dv p 1 2222 1111 1 3333 2 4444 8
$ ./routenode dv p 1 3333 1111 50 2222 2 4444 5
$ ./routenode dv p 1 4444 2222 8 3333 5 last

2-4. Poisoned Reverse mode with Link Cost Change
$ ./routenode dv p 1 1111 2222 1 3333 50
$ ./routenode dv p 1 2222 1111 1 3333 2 4444 8
$ ./routenode dv p 1 3333 1111 50 2222 2 4444 5
$ ./routenode dv p 1 4444 2222 8 3333 5 last 100

3. Link State Routing Protocol - Simple test cases
3-1. Wittout Link Cost Change
$ ./routenode ls r 2 1111 2222 1 3333 50
$ ./routenode ls r 2 2222 1111 1 3333 2 4444 8
$ ./routenode ls r 2 3333 1111 50 2222 2 4444 5
$ ./routenode ls r 2 4444 2222 8 3333 5 last
