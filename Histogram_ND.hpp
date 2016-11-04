#pragma once
#ifndef HISTOGRAM_ND_HPP
#define HISTOGRAM_ND_HPP

#include <cmath>
#include <vector>
#include <functional>
#include <initializer_list>

//=========================================================================
// Example 0
//
// 2-d histogram with double precision, one dimension is int, other is float
// dimension 0 : takes integers with min 0, max 10 and 10 buckets
// dimension 1 : takes floats with min 0, max 10 and 15 buckets
//
// auto histogram2d =
//		Histogram::HistogramUniformND_<double, int, float>(
//			MinMaxBins<int>{0,10,10},
//			MinMaxBins<float>{0,10,15});
//
// histogram.Inc(5, 5.0f);
//
// auto value = histogram.Value(5, 5.0f);
//=========================================================================
// Example 1
//
// 4-d histogram with double precision, one dimension is int,
//		other is float, other is double and final is uint8_t
// dimension 0 : takes integer with min 0, max 100 and 10 buckets
// dimension 1 : takes float with min 0, max 100 and 15 buckets
// dimension 2 : takes double with min 0, max 1000 and 100 buckets
// dimension 3 : takes uint8_t with min 0, max 255 and 5 buckets
//
// auto histogram4d =
//		Histogram::HistogramUniformND_<double, int, float, double, uint8_t>(
//			MinMaxBins<int>{0,100,10},
//			MinMaxBins<float>{0,100,15},
//			MinMaxBins<double>{0,100,15},
//			MinMaxBins<uint32_t>{0,100,15},);
//
// histogram.Inc(5, 50.0f, 88.0d, 1);
//
// auto value = histogram.Value(5, 50.0f, 88.0d, 1);
//
//=========================================================================
namespace Histogram
{
	template<typename T>
	struct MinMaxBins
	{
		T min;
		T max;
		uint32_t bins;
	};

	template<typename Type>
	std::function<uint32_t(Type)>
	FactoryUniformIndexFunction1D_(
			const MinMaxBins<Type>& m)
	{
		if (m.min >= m.max)
		{
			throw std::invalid_argument("min should be < max");
		}

		if (m.bins <= 0)
		{
			throw std::invalid_argument("bins must be > 0");
		}

		return
			[m](Type value)
			{
				if (value <= m.min)
				{
					return uint32_t(0);
				}

				if (value >= m.max)
				{
					return uint32_t(m.bins - 1);
				}

				return uint32_t(std::round(
						double(value) * double(m.bins - 1) / double(m.max - m.min)));
			};
	}

	//=========================================================================

	template<typename T>
	inline uint32_t
	NumberOfBins(
			MinMaxBins<T> minMaxBin)
	{
		return minMaxBin.bins;
	}

	template<typename T, typename... Args>
	inline uint32_t
	NumberOfBins(
			MinMaxBins<T> minMaxBin,
			MinMaxBins<Args>... minMaxBins)
	{
		return minMaxBin.bins * NumberOfBins(minMaxBins...);
	}

	//=========================================================================

	template<typename T>
	inline std::function<uint32_t(T)>
	Factory(MinMaxBins<T> m0)
	{
		auto f0 = FactoryUniformIndexFunction1D_<T>(m0);
		return [f0](T t0){ return f0(t0);};
	}

	template<typename T, typename... Args>
	inline std::function<uint32_t(T, Args...)>
	Factory(MinMaxBins<T> m0, MinMaxBins<Args>... mN)
	{
		auto f0 = FactoryUniformIndexFunction1D_<T>(m0);

		return
			[f0, m0, mN...](T t0, Args... args)
			{
				return f0(t0) + m0.bins * Factory<Args...>(mN...)(args...);
			};
	}

	//=========================================================================

	template<
		typename Precision,
		typename... Types>
	class Histogram_
	{
		protected:
			std::vector<Precision> m_bins;
			std::function<uint32_t(Types...)> m_indexOf = nullptr;
		public:
			//-----------------------------------------------
			Histogram_(
					uint32_t noBins,
					std::function<uint32_t(Types...)> indexFunction)
			{
				if (noBins <= 0)
				{
					throw std::invalid_argument("bins should be > 0");
				}

				if (indexFunction == nullptr)
				{
					throw std::invalid_argument("index function should be > 0");
				}

				m_indexOf = indexFunction;
				m_bins = std::vector<Precision>(noBins, Precision(0));
			}

			//-----------------------------------------------

			Histogram_(
					const Histogram_& histogram) :
						Histogram_(histogram.Bins().size(), histogram.IndexFunction())
			{
				Set(histogram);
			}

			//-----------------------------------------------

			~Histogram_()
			{}

			//-----------------------------------------------

			/*
			 * Apply operation on bins
			 */
			Histogram_<Precision, Types...>&
			Apply(const std::function<Precision(Precision)> func)
			{
				std::for_each(
						m_bins.begin(),
						m_bins.end(),
						[func](Precision& bin)
						{
							bin = func(bin);
						});
				return *this;
			}

			//-----------------------------------------------

			Histogram_& IncMultiplier(Precision multiplier, Types... types)
			{
				auto index = m_indexOf(types...);
				m_bins[index] += multiplier;
				return *this;
			}

			//-----------------------------------------------

			Histogram_<Precision, Types...>&
			Inc(Types... types)
			{
				return IncMultiplier(Precision(1), types...);
			}

			//-----------------------------------------------

			Histogram_<Precision, Types...>&
			Set(Precision value)
			{
				return Apply([value](Precision bin){return value;});
			}

			//-----------------------------------------------

			Histogram_<Precision, Types...>&
			Set(const std::vector<Precision>& bins)
			{
				size_t mBinsSize = m_bins.size();
				size_t binsSize = bins.size();

				if (mBinsSize != binsSize)
				{
					throw std::invalid_argument("bins size not the same");
				}

				for (size_t i = 0; i < mBinsSize; i++)
				{
					m_bins[i] = bins[i];
				}

				return *this;
			}

			//-----------------------------------------------

			Histogram_<Precision, Types...>&
			Set(const Histogram_<Precision, Types...>& histogram)
			{
				return Set(histogram.Bins());
			}

			//-----------------------------------------------

			Histogram_<Precision, Types...>&
			Add(const Precision value)
			{
				return Apply([value](Precision bin){return bin+value;});
			}

			//-----------------------------------------------

			Histogram_<Precision, Types...>&
			Add(const std::vector<Precision>& bins)
			{
				size_t mBinsSize = m_bins.size();
				size_t binsSize = bins.size();

				if (mBinsSize != binsSize)
				{
					throw std::invalid_argument("bins size not the same");
				}

				for (size_t i = 0; i < mBinsSize; i++)
				{
					m_bins[i] += bins[i];
				}

				return *this;
			}

			//-----------------------------------------------

			Histogram_<Precision, Types...>&
			Add(const Histogram_<Precision, Types...>& histogram)
			{
				return Add(histogram.Bins());
			}

			//-----------------------------------------------

			Histogram_<Precision, Types...>&
			Clear()
			{
				return Set(Precision(0));
			}

			//-----------------------------------------------

			const std::vector<Precision>&
			Bins() const
			{
				return m_bins;
			}

			//-----------------------------------------------

			const std::function<uint32_t(Types...)>
			IndexFunction() const
			{
				return m_indexOf;
			}

			//-----------------------------------------------

			Precision
			Value(Types... types) const
			{
				auto index = m_indexOf(types...);
				return m_bins.at(index);
			}

			//-----------------------------------------------

			/*
			 * Sum of all bins values
			 */
			Precision
			Sum() const
			{
				Precision sum = 0;

				for (const auto& bin : m_bins)
				{
					sum += std::abs(bin);
				}

				return sum;
			}

			//-----------------------------------------------

			/*
			 * Normalize bins so values sum up to 1.0
			 */
			Histogram_<Precision, Types...>&
			Normalize()
			{
				Precision sum = Sum();

				if (sum > Precision(0))
				{
					for (auto& bin : m_bins)
					{
						bin /= sum;
					}
				}

				return *this;
			}

			//-----------------------------------------------
			static Histogram_<Precision, Types...>
			Add(std::initializer_list<Histogram_<Precision, Types...>> histograms)
			{
				auto size = histograms.size();

				if (size <= 0)
				{
					throw std::invalid_argument("cannot add zero histograms");
				}

				Histogram_<Precision, Types...> result(histograms[0]);

				for (size_t i = 1; i < size; i++)
				{
					result.Add(histograms[i]);
				}

				return result;
			}
	};

	//=========================================================================

	template<
		typename Precision,
		typename... Types>
	class HistogramUniformND_ :
			public Histogram_<Precision, Types...>
	{
		public:
			HistogramUniformND_(
					MinMaxBins<Types>... mS) :
						Histogram_<Precision, Types...>(
								NumberOfBins(mS...),
								Factory(mS...))
			{}
	};
  
  //=========================================================================
}

#endif
