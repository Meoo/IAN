
# IAN-happy


## TODO

```
NAMESPACE foo.bar

STRUCT MyData
{
  a: int
  b: float
  c: string
}

MAPPING MyData: cpp
{
  c: 'std::string'
}

ENCODING MyData
{
  DELTA
  {
    a: VLE
  }
  DELTA b: VLE
  c
}
```
