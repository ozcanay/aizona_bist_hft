### How to Build

- Setup VCPKG: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell
- Run build task from VSCode 

### TODO

- assert that customer info characters are all printable -> std::isprint. this is from the DOCS.
- Quill custom sink
- AVX for order finding (OUCH) and orderbook level finding (ITCH)
- Dummy strategy
- Profiling tools: RDTSCP. https://stackoverflow.com/questions/27693145/rdtscp-versus-rdtsc-cpuid
- utilize secondary ITCH, sometimes it is faster.
- ITCH-OUCH arb. responses coma quicker on ITCH. we can make use of that.
- Install bist_config contents to build directory.
- Use my types to parse ITCH.

### TODO FOR SERVER SIDE

- Order ID will be unique per side and orderbook ID.
- For same side and orderbook ID if a order ID is encountered twice, report error.
- list<Order>::iterator
- symbol - orderbook map.
- optimized L3 orderbook.
- capture OUCH client request PCAPs and feed them into telnet while testing.

## Related docs:

- https://access.redhat.com/sites/default/files/attachments/201501-perf-brief-low-latency-tuning-rhel7-v2.1.pdf
- Agner Fog
- Everything you need to know about memory -> ulrich drepper
- denis bakhalov book
- https://travisdowns.github.io/blog/2020/08/19/icl-avx512-freq.html

## prereq:

pip install scapy