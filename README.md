# Resource

It's a bit like a shared_ptr, but created on stack.

---

## usage

non-copyable resource (e.g. OpenGL shader wrapper)
```C++
Resource<ShaderProgram> simple_green_shader;
simple_green_shader->use();
```

references
```C++
ResourceReference<ShaderProgram> ref = &simple_green_shader;
ref->setSomething();
ref->doSomething();

...

// if Resource still exists
if(ref)
	ref->doSomething();
```

---

The .natvis file is for VisualStudio debug view, just include it in project. [more about natvis &nearr;](https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects)

the pattern is, for `Resource<T>`: [num of references] = { T }
<br>and for `ResourceReference<T>`: [num of references] -> { T }