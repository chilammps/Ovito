// Construction
tm = new AffineTransformation()

// Printing
print(tm)
print(tm.toString())

// Element read access
assert(tm.element(0,0) == 1)
assert(tm.element(1,0) == 0)

// Element write access
tm.setElement(0,2,0.5)
assert(tm.element(0,2) == 0.5)

// Column access
print(tm.column(1))
assert(tm.column(1).equals([0,1,0]))
tm.setColumn(1, [1,2,3])
assert(tm.column(1).equals([1,2,3]))
print(tm)