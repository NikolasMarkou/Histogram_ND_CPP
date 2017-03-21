# Histogram_ND_CPP
A modern C++11 header only implementation of a generic n-dimensional histogram with no dependencies.

Important to note is that you can mix types, mix min and max limits and mix bin counts, you can even extend for non uniform bucketing.
Most operations can also be chained to avoid tedious typing

To use just drop into your project and call like in the following examples

## Example 0
One dimensional histogram with double precision, one dimension is float
* dimension 0 : takes float with min -10000, max 10000 and 10 buckets

```cpp
auto histogram1d =
	HistogramUniformND_<double, float>(
			MinMaxBins<float>{-10000,+10000,10});

histogram1d
  .Inc(5.0f)
  .Inc(100.0f)
  .Inc(1990.0f)
  .Inc(3000.0f)
  .Inc(300000.0f)
  .Normalize();

auto value = histogram1d.Value(20.0f);
```

## Example 1
Two dimensional histogram with double precision, one dimension is int, other is float
* dimension 0 : takes integers with min 0, max 10 and 10 buckets
* dimension 1 : takes floats with min 0, max 10 and 15 buckets

```cpp
auto histogram2d =
	HistogramUniformND_<double, int, float>(
			MinMaxBins<int>{0,10,10},
			MinMaxBins<float>{0,10,15});

histogram2d.Inc(5, 5.0f);

auto value = histogram2d.Value(5, 5.0f);
```

## Example 2
Four dimensional histogram with double precision, one dimension is int, other is float, other is double and final is uint8_t
* dimension 0 : takes integer with min 0, max 100 and 10 buckets
* dimension 1 : takes float with min 0, max 100 and 15 buckets
* dimension 2 : takes double with min 0, max 1000 and 100 buckets
* dimension 3 : takes uint8_t with min 0, max 255 and 5 buckets

```cpp
auto histogram4d =
		Histogram::HistogramUniformND_<double, int, float, double, uint8_t>(
			MinMaxBins<int>{0,100,10},
			MinMaxBins<float>{0,100,15},
			MinMaxBins<double>{0,100,15},
			MinMaxBins<uint32_t>{0,100,15});

histogram4d.Inc(5, 50.0f, 88.0d, 1);

auto value = histogram4d.Value(5, 50.0f, 88.0d, 1);
```

## Example 3

Three dimensional color histogram, single precision
* dimension 0 : takes uint8_t with min 0, max 255 and 10 buckets
* dimension 1 : takes uint8_t with min 0, max 255 and 10 buckets
* dimension 2 : takes uint8_t with min 0, max 255 and 10 buckets

```cpp
auto histogram3dRGB =
		Histogram::HistogramUniformND_<float, uint8_t, uint8_t, uint8_t>(
			MinMaxBins<uint8_t>{0,255,10},
			MinMaxBins<uint8_t>{0,255,10},
			MinMaxBins<uint8_t>{0,255,10});

histogram3dRGB.Inc(100, 100, 200);

auto value = histogram3dRGB.Value(100, 100, 200);
```
