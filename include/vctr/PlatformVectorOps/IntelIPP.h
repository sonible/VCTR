/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2022- by sonible GmbH.

    This file is part of VCTR - Versatile Container Templates Reconceptualized.

    VCTR is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    VCTR is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License version 3 for more details.

    You should have received a copy of the GNU Lesser General Public License
    version 3 along with VCTR.  If not, see <https://www.gnu.org/licenses/>.
  ==============================================================================
*/

namespace vctr::PlatformVectorOps
{

// clang-format off

template <class T>
class IntelIPP
{
};

#if VCTR_USE_IPP


template <IppStatus... allowedStatus>
void assertAllowedStatus ([[maybe_unused]] IppStatus s)
{
    VCTR_ASSERT ((s == allowedStatus) || ...);
}

inline void assertIppNoErr ([[maybe_unused]] IppStatus s)
{
    assertAllowedStatus<ippStsNoErr> (s);
}

template <>
class IntelIPP<float>
{
public:
    static void abs (const float* src,                     float* dst, int len) { assertIppNoErr (ippsAbs_32f (src, dst, len)); }
    static void add (const float* srcA, const float* srcB, float* dst, int len) { assertIppNoErr (ippsAdd_32f (srcA, srcB, dst, len)); }
    static void add (const float* srcA, float srcB,        float* dst, int len) { assertIppNoErr (ippsAddC_32f (srcA, srcB, dst, len)); }
    static void sub (const float* srcA, const float* srcB, float* dst, int len) { assertIppNoErr (ippsSub_32f (srcB, srcA, dst, len)); }
    static void sub (const float* srcA, float srcB,        float* dst, int len) { assertIppNoErr (ippsSubC_32f (srcA, srcB, dst, len)); }
    static void sub (float srcA,        const float* srcB, float* dst, int len) { assertIppNoErr (ippsSubCRev_32f (srcB, srcA, dst, len)); }
    static void mul (const float* srcA, const float* srcB, float* dst, int len) { assertIppNoErr (ippsMul_32f (srcA, srcB, dst, len)); }
    static void mul (const float* srcA, float srcB,        float* dst, int len) { assertIppNoErr (ippsMulC_32f (srcA, srcB, dst, len)); }
    static void mul (float srcA,        float* srcDst,                 int len) { assertIppNoErr (ippsMulC_32f_I (srcA, srcDst, len)); }
    static void div (const float* srcA, const float* srcB, float* dst, int len) { assertIppNoErr (ippsDiv_32f (srcB, srcA, dst, len)); }
    static void div (const float* srcA, float srcB,        float* dst, int len) { assertIppNoErr (ippsDivC_32f (srcA, srcB, dst, len)); }
    static void div (float srcA,        const float* srcB, float* dst, int len) { assertIppNoErr (ippsDivCRev_32f (srcB, srcA, dst, len)); }

    static void multiplyAccumulate (const float* srcA, const float* srcB, float* srcDst, int len) { assertIppNoErr (ippsAddProduct_32f (srcA, srcB, srcDst, len)); }
    static void multiplyAccumulate (const float* srcA, float        srcB, float* srcDst, int len) { assertIppNoErr (ippsAddProductC_32f (srcA, srcB, srcDst, len)); }

    static void clampLow  (const float* src, float thresh, float* dst, int len) { assertIppNoErr (ippsThreshold_LT_32f (src, dst, len, thresh)); }
    static void clampHigh (const float* src, float thresh, float* dst, int len) { assertIppNoErr (ippsThreshold_GT_32f (src, dst, len, thresh)); }

    static void ln    (const float* src, float* dst, int len) { assertAllowedStatus<ippStsNoErr, ippStsSingularity> (ippsLn_32f (src, dst, len)); }
    static void log10 (const float* src, float* dst, int len) { assertAllowedStatus<ippStsNoErr, ippStsSingularity> (ippsLog10_32f_A24 (src, dst, len)); }
    static void exp   (const float* src, float* dst, int len) { assertIppNoErr (ippsExp_32f (src, dst, len)); }

    static float max    (const float* src, int len) { float r; assertIppNoErr (ippsMax_32f (src, len, &r)); return r; }
    static float maxAbs (const float* src, int len) { float r; assertIppNoErr (ippsMaxAbs_32f (src, len, &r)); return r; }
    static float min    (const float* src, int len) { float r; assertIppNoErr (ippsMin_32f (src, len, &r)); return r; }
    static float minAbs (const float* src, int len) { float r; assertIppNoErr (ippsMinAbs_32f (src, len, &r)); return r; }
    static float sum    (const float* src, int len) { float r; assertIppNoErr (ippsSum_32f (src, len, &r, ippAlgHintNone)); return r; }
    static float mean   (const float* src, int len) { float r; assertIppNoErr (ippsMean_32f (src, len, &r, ippAlgHintNone)); return r;}
    static float l2Norm (const float* src, int len) { float r; assertIppNoErr (ippsNorm_L2_32f (src, len, &r)); return r; }
};

template <>
class IntelIPP<double>
{
public:
    static void abs (const double* src,                      double* dst, int len) { assertIppNoErr (ippsAbs_64f (src, dst, len)); }
    static void add (const double* srcA, const double* srcB, double* dst, int len) { assertIppNoErr (ippsAdd_64f (srcA, srcB, dst, len)); }
    static void add (const double* srcA, double srcB,        double* dst, int len) { assertIppNoErr (ippsAddC_64f (srcA, srcB, dst, len)); }
    static void sub (const double* srcA, const double* srcB, double* dst, int len) { assertIppNoErr (ippsSub_64f (srcB, srcA, dst, len)); }
    static void sub (const double* srcA, double srcB,        double* dst, int len) { assertIppNoErr (ippsSubC_64f (srcA, srcB, dst, len)); }
    static void sub (double srcA,        const double* srcB, double* dst, int len) { assertIppNoErr (ippsSubCRev_64f (srcB, srcA, dst, len)); }
    static void mul (const double* srcA, const double* srcB, double* dst, int len) { assertIppNoErr (ippsMul_64f (srcA, srcB, dst, len)); }
    static void mul (const double* srcA, double srcB,        double* dst, int len) { assertIppNoErr (ippsMulC_64f (srcA, srcB, dst, len)); }
    static void mul (double srcA,        double* srcDst,                  int len) { assertIppNoErr (ippsMulC_64f_I (srcA, srcDst, len)); }
    static void div (const double* srcA, const double* srcB, double* dst, int len) { assertIppNoErr (ippsDiv_64f (srcB, srcA, dst, len)); }
    static void div (const double* srcA, double srcB,        double* dst, int len) { assertIppNoErr (ippsDivC_64f (srcA, srcB, dst, len)); }

    static void multiplyAccumulate (const double* srcA, const double* srcB, double* srcDst, int len) { assertIppNoErr (ippsAddProduct_64f (srcA, srcB, srcDst, len)); }
    static void multiplyAccumulate (const double* srcA, double        srcB, double* srcDst, int len) { assertIppNoErr (ippsAddProductC_64f (srcA, srcB, srcDst, len)); }

    static void clampLow  (const double* src, float thresh,  double* dst, int len) { assertIppNoErr (ippsThreshold_LT_64f (src, dst, len, thresh)); }
    static void clampHigh (const double* src, float thresh,  double* dst, int len) { assertIppNoErr (ippsThreshold_GT_64f (src, dst, len, thresh)); }

    static void ln    (const double* src, double* dst, int len) { assertAllowedStatus<ippStsNoErr, ippStsSingularity> (ippsLn_64f (src, dst, len)); }
    static void log10 (const double* src, double* dst, int len) { assertAllowedStatus<ippStsNoErr, ippStsSingularity> (ippsLog10_64f_A53 (src, dst, len)); }
    static void exp   (const double* src, double* dst, int len) { assertIppNoErr (ippsExp_64f (src, dst, len)); }

    static double max    (const double* src, int len) { double r; assertIppNoErr (ippsMax_64f (src, len, &r)); return r; }
    static double maxAbs (const double* src, int len) { double r; assertIppNoErr (ippsMaxAbs_64f (src, len, &r)); return r; }
    static double min    (const double* src, int len) { double r; assertIppNoErr (ippsMin_64f (src, len, &r)); return r; }
    static double minAbs (const double* src, int len) { double r; assertIppNoErr (ippsMinAbs_64f (src, len, &r)); return r; }
    static double sum    (const double* src, int len) { double r; assertIppNoErr (ippsSum_64f (src, len, &r)); return r; }
    static double mean   (const double* src, int len) { double r; assertIppNoErr (ippsMean_64f (src, len, &r)); return r;}
    static double l2Norm (const double* src, int len) { double r; assertIppNoErr (ippsNorm_L2_64f (src, len, &r)); return r;}
};

template <>
class IntelIPP<std::complex<float>>
{
private:
    static auto fc (std::complex<float>* c)       { return reinterpret_cast<Ipp32fc*> (c); }
    static auto fc (const std::complex<float>* c) { return reinterpret_cast<const Ipp32fc*> (c); }
    static auto fc (std::complex<float> c)        { return *fc (&c); }
    
public:
    static void abs (const std::complex<float>* src,                                   float* dst,               int len) { assertIppNoErr (ippsAbs_32fc_A24 (fc (src), dst, len)); }
    static void add (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* dst, int len) { assertIppNoErr (ippsAdd_32fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void add (const std::complex<float>* srcA, std::complex<float> srcB,        std::complex<float>* dst, int len) { assertIppNoErr (ippsAddC_32fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void sub (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* dst, int len) { assertIppNoErr (ippsSub_32fc (fc (srcB), fc (srcA), fc (dst), len)); }
    static void sub (const std::complex<float>* srcA, std::complex<float> srcB,        std::complex<float>* dst, int len) { assertIppNoErr (ippsSubC_32fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void sub (std::complex<float> srcA,        const std::complex<float>* srcB, std::complex<float>* dst, int len) { assertIppNoErr (ippsSubCRev_32fc (fc (srcB), fc (srcA), fc (dst), len)); }
    static void mul (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* dst, int len) { assertIppNoErr (ippsMul_32fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void mul (const std::complex<float>* srcA, std::complex<float> srcB,        std::complex<float>* dst, int len) { assertIppNoErr (ippsMulC_32fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void div (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* dst, int len) { assertIppNoErr (ippsDiv_32fc (fc (srcB), fc (srcA), fc (dst), len)); }
    static void div (const std::complex<float>* srcA, std::complex<float> srcB,        std::complex<float>* dst, int len) { assertIppNoErr (ippsDivC_32fc (fc (srcA), fc (srcB), fc (dst), len)); }

    static void multiplyAccumulate (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* srcDst, int len) { assertIppNoErr (ippsAddProduct_32fc (fc (srcA), fc (srcB), fc (srcDst), len)); }

    static void conj          (const std::complex<float>* src, std::complex<float>* dst, int len) { assertIppNoErr (ippsConj_32fc (fc (src), fc (dst), len)); }
    static void angle         (const std::complex<float>* src, float* dst, int len)               { assertIppNoErr (ippsPhase_32fc (fc (src), dst, len)); }
    static void powerSpectrum (const std::complex<float>* src, float* dst, int len)               { assertIppNoErr (ippsPowerSpectr_32fc (fc (src), dst, len)); }

    static std::complex<float> sum  (const std::complex<float>* src, int len) { std::complex<float> r; assertIppNoErr (ippsSum_32fc (fc (src), len, fc (&r), ippAlgHintNone)); return r; }
    static std::complex<float> mean (const std::complex<float>* src, int len) { std::complex<float> r; assertIppNoErr (ippsMean_32fc (fc (src), len, fc (&r), ippAlgHintNone)); return r;}
};

template <>
class IntelIPP<std::complex<double>>
{
private:
    static auto fc (std::complex<double>* c)       { return reinterpret_cast<Ipp64fc*> (c); }
    static auto fc (const std::complex<double>* c) { return reinterpret_cast<const Ipp64fc*> (c); }
    static auto fc (std::complex<double> c)        { return *fc (&c); }

public:
    static void abs (const std::complex<double>* src,                                    double* dst,               int len) { assertIppNoErr (ippsAbs_64fc_A53 (fc (src), dst, len)); }
    static void add (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* dst, int len) { assertIppNoErr (ippsAdd_64fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void add (const std::complex<double>* srcA, std::complex<double> srcB,        std::complex<double>* dst, int len) { assertIppNoErr (ippsAddC_64fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void sub (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* dst, int len) { assertIppNoErr (ippsSub_64fc (fc (srcB), fc (srcA), fc (dst), len)); }
    static void sub (const std::complex<double>* srcA, std::complex<double> srcB,        std::complex<double>* dst, int len) { assertIppNoErr (ippsSubC_64fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void sub (std::complex<double> srcA,        const std::complex<double>* srcB, std::complex<double>* dst, int len) { assertIppNoErr (ippsSubCRev_64fc (fc (srcB), fc (srcA), fc (dst), len)); }
    static void mul (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* dst, int len) { assertIppNoErr (ippsMul_64fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void mul (const std::complex<double>* srcA, std::complex<double> srcB,        std::complex<double>* dst, int len) { assertIppNoErr (ippsMulC_64fc (fc (srcA), fc (srcB), fc (dst), len)); }
    static void div (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* dst, int len) { assertIppNoErr (ippsDiv_64fc (fc (srcB), fc (srcA), fc (dst), len)); }
    static void div (const std::complex<double>* srcA, std::complex<double> srcB,        std::complex<double>* dst, int len) { assertIppNoErr (ippsDivC_64fc (fc (srcA), fc (srcB), fc (dst), len)); }

    static void multiplyAccumulate (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* srcDst, int len) { assertIppNoErr (ippsAddProduct_64fc (fc (srcA), fc (srcB), fc (srcDst), len)); }

    static void conj          (const std::complex<double>* src, std::complex<double>* dst, int len) { assertIppNoErr (ippsConj_64fc (fc (src), fc (dst), len)); }
    static void angle         (const std::complex<double>* src, double* dst, int len)               { assertIppNoErr (ippsPhase_64fc (fc (src), dst, len)); }
    static void powerSpectrum (const std::complex<double>* src, double* dst, int len)               { assertIppNoErr (ippsPowerSpectr_64fc (fc (src), dst, len)); }

    static std::complex<double> sum  (const std::complex<double>* src, int len) { std::complex<double> r; assertIppNoErr (ippsSum_64fc (fc (src), len, fc (&r))); return r; }
    static std::complex<double> mean (const std::complex<double>* src, int len) { std::complex<double> r; assertIppNoErr (ippsMean_64fc (fc (src), len, fc (&r))); return r;}
};

template <>
class IntelIPP<int32_t>
{
public:
    static auto abs (const int32_t* src, int32_t* dst, int len) { assertIppNoErr (ippsAbs_32s (src, dst, len)); }
};
#endif

// clang-format on

} // namespace vctr::PlatformVectorOps
