// Construction
v1 = new Vector(1,2,3)
v2 = new Vector([4, 5, 6.0])

// Printing
print(v1)
print(v2.toString())

// Component read access
print("v1: x=", v1.x, "y=", v1.y, "z=", v1.z)

// Component write access
v1.x = -1; v1.y = 0; v1.z = 23;
print("v1 = " + v1)

// Vector algebra
print("plus: " + v1 + " + " + v2 + " = " + v1.plus(v2))
print("plus: " + v1 + " + " + [1,1,1] + " = " + v1.plus([1,1,1]))
print("minus: " + v1 + " - " + [1,2,3] + " = " + v1.minus([1,2,3]))

// Dot and cross products
print("dot: " + Vector(1,2,3).dot([0,-2,1]))
print("cross: " + Vector(1,0,0).cross(Vector(0,1,0)))

// Conversion to JavaScript array
print("toArray: " + v1.toArray())
