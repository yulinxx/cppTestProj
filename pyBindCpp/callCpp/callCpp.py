import example

print("abc")
dir(example)

example.__doc__
example.__file__
print(example.add(1, 2))
print(example.say("AAA"))

print('\n-------------\n')

p = example.Pet("MiaoMiao")
print(p)
print(p.getName())

p.setName("WangWang")
print(p.getName())

print('\n-------------\n')
# 创建 MyClass 对象
obj = example.MyClass(42)

# 调用 getData 方法
print(obj.getData())  # 输出: 42

# 调用 setData 方法
obj.setData(100)

# 再次调用 getData 方法
print(obj.getData())  # 输出: 100

print('\n-------------\n')

# 创建 ComplexNumber 对象
c = example.ComplexNumber(3.0, 4.0)

# 调用成员函数
print(c.getRealPart())         # 输出: 3.0
print(c.getImaginaryPart())    # 输出: 4.0
print(c.getMagnitude())        # 输出: 5.0

# 修改成员变量
c.setRealPart(1.0)
c.setImaginaryPart(2.0)

# 再次调用成员函数
print(c.getRealPart())         # 输出: 1.0
print(c.getImaginaryPart())    # 输出: 2.0
print(c.getMagnitude())        # 输出: 2.23606797749979

print('\n-------------\n')


# 创建 Point 对象
p = example.Point(1.0, 2.0)

# 创建 Circle 对象，并传入 Point 对象作为参数
c = example.Circle(5.0, p)

# 调用 Point 的成员函数
print("Point X:", c.getCenter().getX())  # 输出: 1.0
print("Point Y:", c.getCenter().getY())  # 输出: 2.0

# 调用 Circle 的成员函数
print("Circle Radius:", c.getRadius())   # 输出: 5.0

print('\n-------------\n')
print('\n-------------\n')
print('\n-------------\n')