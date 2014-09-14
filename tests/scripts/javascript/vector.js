// Construction
v1 = new Vector(1,2,3)
v2 = new Vector([4, 5, 6])

// Printing
print(v1)
print(v2.toString())

// Component read access
assert(v1.x == 1)
assert(v1.y == 2)
assert(v1.z == 3.0)

// Component write access
v3 = new Vector(10,20,30)
v3.x = -1; v3.y = 0; v3.z = 23.5;
assert(v3 == Vector(-1,0,23.5))

// Vector algebra
assert(v1.plus(v2) == Vector(5,7,9))
assert(v1.plus([1,1,1]) == Vector(2,3,4))
assert(v1.minus(v2) == Vector(-3,-3,-3))

// Dot and cross products
assert(Vector(1,2,3).dot([0,-2,1]) == -1)
assert(Vector(1,0,0).cross(Vector(0,1,0)) == Vector(0,0,1))
assert(Vector(1,0,0).cross([0,1,0]) == Vector(0,0,1))

// Conversion to JavaScript array
assert(v1.toArray().length == 3)
assert(v1.toArray()[2] == 3.0)
