Calculus Vaporis CPU Design
===========================

This is a sketch of a design for a very small zero-operand 12-bit CPU
in about 1000 NAND gates, or a smaller number of more powerful
components (e.g. 70 lines of C).  It’s my first CPU design, so it may
be deeply flawed.

Initially I called it the “Dumbass CPU”, but I thought that didn’t
seem like a name Charles Babbage would have used.  So I called it
“Calculus Vaporis”, which means “counting-stone of steam” in Latin, I
hope.

This directory contains a simulator written in C, a few simple
programs in a simple assembly language, and an assembler written in
Python.  Run `make` to try them out.

Logical Organization
--------------------

The machine has four 12-bit registers: P, A, X, and I.

P is the program counter.  It generally holds the address of the next
instruction to fetch.  It could actually be only 11 bits.

A is the top-of-stack register.

X is the second-on-stack register.

I is the instruction-decoding register.  It holds the instruction word
currently being decoded.

The machine additionally has an 11-bit address bus B, a 12-bit data
bus D, and a bus operation O, which are treated as pseudo-registers
for the purpose of the RTL description. The narrow memory bus is weird
but simplifies the usage of immediate constants.

Instruction Set
---------------

The machine cycle has four microcycles: fetch 1, fetch 2, execute 1,
execute 2.  Fetch 1 and 2 are the same for all instructions.  All
instructions but @ do nothing during execute 2.  The other RTL below
explains what happens during execute 1.

Following Python, I am using `R[-1]` to refer to the highest bit of
register `R`, `R[:-1]` to refer to all but the highest bit, `R[-2]` to
refer to the next-highest bit, and so on.

There are seven instructions: `$`, `.`, `-`, `|`, `@`, `!`, and `nop`.
They are stored one per machine word, which is grossly wasteful of
memory space but makes for a simpler instruction decode cycle.
Avoiding the gross waste of memory space would probably require adding
at least one more register to the processor, because as it is defined
now, roughly half the instructions in any code are `$`, which needs a
whole machine word most of the time anyway.

There are even simpler instruction sets, such as the subtract-
indirect-and-branch-indirect-if-negative OISC, and the MOV machine.
While these instruction sets are simpler to explain, the existing
decoding logic is fairly small, about 20 out of the 1000 NAND gates.
These simpler instruction sets call for a more complicated machine
cycle.

- `$` is the “load immediate” instruction; it’s represented by having a
    single high bit set in a 12-bit word.  When written in code
    sequences, it is followed by the value of the other 11 bits.  It
    does:

    >     X ← A, A[:-1] ← I[:-1], A[-1] ← I[-2]

    Note that this is the only way to change the value of X, by copying
    A’s old value into it.

- `.` is the “conditional call/jump” instruction.  It does:

    >     if X[-1] == 0: 
    >         P ← A[:-1], A[:-1] ← P, A[-1] ← 0;
    >     else:
    >         A ← X;  # not very important, kind of arbitrary, maybe drop it

- `-` is the “subtract” instruction.  It does:

    >     A ← X - A;

- `|` is the “NAND” instruction (named after the Sheffer stroke).  In C
    notation for bitwise operators, it does:

    >     A ← ~(A & X);

- `@` is the “fetch” instruction.  It is the only instruction that needs
    both execute microcycles.  During execute 1, it does:

    >     B ← A[:-1], O ← “read”;

    Then, during execute 2:

    >     A ← D;

- `!` is the “store” instruction.  It does:

    >     B ← A[:-1], D ← X, O ← “write”, A ← X;

- `nop` does nothing.

Code Snippets
-------------

These show that the instruction set is usable, just barely.

Call the subroutine at address `x`, passing the return address in A:

    $0 $x .

Store the return address passed in A at a fixed address `ra` (for
non-reentrant subroutines):

    $ra !

Return to the address thus passed, trashing both A and X:

    $0 $ra @ .

Decrement the memory location `sp`, used as, for example, a stack
pointer:

    $sp @ $1 - $sp !

Store a return address at the location pointed to by `sp` after
decrementing `sp`, using an address `tmp` for temporary storage:

    $tmp !  $sp @ $1 - $sp !  $tmp @ $sp !

Fetch that return address from the stack, increment `sp`, and return
to the address:

    $sp @ @ $tmp !  $sp @ $-1 - $sp !  $0 $tmp @ .

Negate the value stored at a memory location `var`:

    $0 $var @ - $var !

Add the values stored at memory locations `a` and `b` with the aid of
a third temporary location `tmp`, leaving the sum in the A register:

    $0 $a @ - $tmp ! $b @ $tmp @ -

The rest of the RTL
-------------------

The instruction definitions above define what happens at the RTL level
during the instruction execution microcycles.  The RTL for the other
two microcycles of the machine cycle follow:

Fetch 1:

    > B ← P, P ← P + 1, O ← “read”;

Fetch 2:

    > I ← D;

The presumed memory interface semantics are:

When O ← “read”, on the next microcycle:

    > D ← M[B];

When O ← “write”:

    >  M[B] ← D.

I don’t know how enough about memory to know how realistic that is.

Translating the RTL design into gates
-------------------------------------

(This part contains a number of errors.  Hopefully it’s accurate
enough that the number of gates can be meaningfully estimated.)

We need a multiplexer attached to the input of most of the registers
to implement the RTL described earlier.  Here are the places each
thing can come from, and when:

<table>
 <tr> <th> register <th> is set from   <th> when
 <tr> <td> A <td> I[:-1] sign-extended <td> ($, execute 1)
 <tr> <td>   <td> P                    <td> (., execute 1, if X[-1] == 0)
 <tr> <td>   <td> X - A                <td> (-, execute 1)
 <tr> <td>   <td> !(X & A)             <td> (|, execute 1)
 <tr> <td>   <td>       D              <td> (@, execute 2)
 <tr> <td>   <td>       X              <td> (!, execute 1; or ., execute 1, when X[-1] == 1)
 <tr> <td> X <td> A                    <td> ($, execute 1)
 <tr> <td> P <td> P + 1                <td> (fetch 1)
 <tr> <td>   <td> A                    <td> (., execute 1, if X[-1] == 0)
 <tr> <td> I <td> D                    <td> (fetch 2)
 <tr> <td> B <td> P                    <td> (fetch 1)
 <tr> <td>   <td> A[:-1]               <td> (@ or !, execute 1)
 <tr> <td> D <td> X                    <td> (!, execute 1)
 <tr> <td> O <td> “read”               <td> (fetch 1, or @, execute 1)
 <tr> <td>   <td> "write"              <td> (!, execute 1)
</table>

At all other times, registers continue with their current values.

The overall design, then, looks something like this.  Some of the
“wires” are N bits wide:

    machine():
        register(P_output, P_input, P_write_enable)
        register(A_output, A_input, A_write_enable)
        register(X_output, X_input, X_write_enable)
        register(I_output, I_input, I_write_enable)
        instruction_decoder(I_output[-4:], fetch_enable, instruction_select)
        # everything up to execute_1 is the inputs; everything after
        # that is an output
        execute_1_controller(instruction_select, fetch_enable, X_output[-1],
                             execute_1, A_select, jump, memory_write, 
                             send_A_to_B, X_write_enable, D_write_enable)

        # All of these are outputs                             
        microcycle_counter(execute_1, execute_2, fetch_1, fetch_2)
        fetch_2 = I_write_enable  # that is, they’re two names for the same wire
        # the last argument is the output
        AND(fetch_enable, execute_2, get_A_from_D)
        OR_6input(get_A_from_D, ..., A_write_enable)
        A_input_mux(A_select, get_A_from_D, 
                    I_sign_extended, P_output, X_minus_A, X_nand_A, D, X_output, 
                    A_input)

        increment_P = fetch_1
        P_input_controller(jump, increment_P, A_output, B_output, P_input)
        OR(fetch_1, ???, memory_read)

        I_sign_extended = I[-2] || I[:-1]

Most of these pieces are simple multiplexers or registers of one sort
or another.  The `microcycle_counter` is just a 4-bit ring counter;
the `instruction_decoder` is just a 6-way decoder of its 4-bit input.
However, the `execute_1_controller` requires a little more
clarification.

    execute_1_controller(instruction_select, instruction_is_fetch, X_highbit,
                         execute_1, A_select, jump, memory_write, 
                         send_A_to_B, X_write_enable, D_write_enable):
        # The A_select output is 5 bits; it’s used to determine where
        # the A register gets read from if it gets read during the
        # execute_1 microcycle.  We don’t care what value it has the
        # rest of the time.
        A_select = (get_A_from_I_sign_extended, get_A_from_P,
                    get_A_from_X_minus_A, get_A_from_X_nand_A,
                    get_A_from_X)
        # The instruction_select input is also 5 bits, representing
        # the five instructions that can affect A, other than fetch.
        instruction_select = (instruction_is_immediate, instruction_is_jump, 
                              instruction_is_subtract, instruction_is_nand,
                              instruction_is_store)

        # Here’s how those outputs are computed.                                        
        get_A_from_I_sign_extended = instruction_is_immediate
        NOT(X_highbit, X_not_highbit)
        AND(instruction_is_jump, X_not_highbit, get_A_from_P)
        get_A_from_X_minus_A = instruction_is_subtract
        get_A_from_X_nand_A = instruction_is_nand
        AND(instruction_is_jump, X_highbit, failed_jump)
        OR(failed_jump, instruction_is_store, get_A_from_X)

        jump = get_A_from_P
        AND(execute_1, instruction_is_store, memory_write)

        OR(instruction_is_store, instruction_is_fetch, memory_access)
        AND(execute_1, memory_access, send_A_to_B)

        AND(execute_1, instruction_is_immediate, X_write_enable)
        
        AND(execute_1, instruction_is_store, D_write_enable)

The ALU simply has to compute `X_minus_A` and `X_nand_A`, which
constantly feed into the `A_input_mux`.  `X_minus_A` requires 12 full
subtractors (analogous to full adders) and `X_nand_A` requires 12 NAND
gates.

Probable errors:

- I wasn’t clear who was responsible for setting the write enables on
  the various registers.
- Some of the `execute_1_controller` outputs probably need to be zero
  when `execute_1` is zero.
- I don’t know anything about memory interfaces and so the memory
  controller is omitted entirely.

Size Estimate
-------------

My initial estimates for number of 2-input NAND gates were:

<table><tr><td>                    <th> bit-serial <th> 12-bit parallel
       <tr><th> microcycle counter <td> 28 NANDs   <td> 28
       <tr><th> rest of control    <td> 127        <td> 578
       <tr><th> registers          <td> 384        <td> 260 (no shifting needed)
       <tr><th> subtractor         <td> 25         <td> 204
       <tr><th> NAND               <td> 1          <td> 12
       <tr><th> bit counter        <td> 70         <td> 0
       <tr><th> total              <td> 635        <td> 1082
</table>

I was estimating a 5-gate D latch per bit in the parallel case, or an
8-gate-per-bit master-slave D flip-flop per bit in the serial case.
My microcycle counter was going to be a pair of D flip-flops, four AND
gates to compute the output bits, and an OR on two of those outputs to
compute the new high bit.

I figured on a ripple-carry subtractor that would cost about 17 NAND
gates per output bit, although I didn’t actually design one.

Because of the relative paucity of N-bit-wide data paths, going
bit-serial doesn’t actually save many gates, but it would slow the
machine down by a substantial factor.

Possible Improvements
---------------------

Dropping the get-X-from-A path on skipped jumps would simplify the
processor, probably without making it any harder to use.

A third register (or more) on the stack wouldn’t affect the
instruction set at all, but would simplify some code.  For example,
the code for “add the values stored at memory locations `a` and `b`,
leaving the sum in the A register” would simplify from:

    $0 $a @ - $tmp ! $b @ $tmp @ -

to:

    $a @ $0 $b @ - -

Using one-hot encodings of the instructions would require using seven
bits of the I register instead of four, but would almost eliminate the
instruction decoder.  (You'd still need to ensure that the instruction
wasn’t $.)

Alternatively, you could pack three, four, or five instructions into
each 12-bit word of memory, as Chuck Moore’s c18 core does, instead of
one.  The "I" instruction, if encodable in lower-order positions,
could simply sign-extend more bits, reducing the number of encodable
immediate constants in such a case, but not to zero.

You could try a different bit width. 2048 instructions of code is
close to a minimum to run, say, a BASIC interpreter.

If N × N → N bit LUTs and registers are available as primitives, the
total complexity of the device could drop by an order of magnitude.
For example, with 5 × 5 → 5 bit LUTs, you could construct a 4-bit
subtractor with borrow-in and borrow-out as a single LUT, and chain
three of them together to get a 12-bit subtractor.  Even if you have
only 4 → 1 bit LUTs, like on a normal FPGA, you can implement a full
subtractor in two LUTs attached to the same inputs, instead of 17 (or
however many) discrete NAND gates.

If the stack were a little deeper, a “dup”, “swap”, or “over”
instruction might help a lot with certain code sequences by
reducing the number of immediate constants.

<link rel="stylesheet" href="http://canonical.org/~kragen/style.css" />
