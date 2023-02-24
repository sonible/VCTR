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

template <class T>
class AppleAccelerate
{
};

#if VCTR_APPLE

template <>
class AppleAccelerate<float>
{
public:
    //==============================================================================
    // vForce functions
    //==============================================================================
    // clang-format off
    static void abs   (const float* src, float* dst, int len) { vvfabsf (dst, src, &len); }
    static void ln    (const float* src, float* dst, int len) { vvlogf (dst, src, &len); }
    static void log10 (const float* src, float* dst, int len) { vvlog10f (dst, src, &len); }
    static void log2  (const float* src, float* dst, int len) { vvlog2f (dst, src, &len); }
    static void exp   (const float* src, float* dst, int len) { vvexpf (dst, src, &len); }
    static void exp2  (const float* src, float* dst, int len) { vvexp2f (dst, src, &len); }
    // clang-format on

    //==============================================================================
    // vDSP functions
    //==============================================================================
    // clang-format off
    static void abs  (const float* src,                          float* dst, size_t len) { vDSP_vabs (src, 1, dst, 1, len); }
    static void add  (const float* srcA, const float* srcB,      float* dst, size_t len) { vDSP_vadd (srcA, 1, srcB, 1, dst, 1, len); }
    static void add  (const float* srcA, float srcB,             float* dst, size_t len) { vDSP_vsadd (srcA, 1, &srcB, dst, 1, len); }
    static void sub  (const float* srcA, const float* srcB,      float* dst, size_t len) { vDSP_vsub (srcB, 1, srcA, 1, dst, 1, len); }
    static void mul  (const float* srcA, const float* srcB,      float* dst, size_t len) { vDSP_vmul (srcA, 1, srcB, 1, dst, 1, len); }
    static void mul  (const float* srcA, float srcB,             float* dst, size_t len) { vDSP_vsmul (srcA, 1, &srcB, dst, 1, len); }
    static void div  (const float* srcA, const float* srcB,      float* dst, size_t len) { vDSP_vdiv (srcB, 1, srcA, 1, dst, 1, len); }
    static void div  (const float* srcA, float srcB,             float* dst, size_t len) { vDSP_vsdiv (srcA, 1, &srcB, dst, 1, len); }

    static void multiplyAdd (const float* srcA, float srcB, float srcC, float* dst, size_t len)              { vDSP_vsmsa (srcA, 1, &srcB, &srcC, dst, 1, len); }
    static void multiplyAdd (const float* srcA, const float* srcB, const float* srcC, float* dst, size_t len) { vDSP_vma (srcA, 1, srcB, 1, srcC, 1, dst, 1, len); }
    static void multiplyAdd (const float* srcA, float srcB,        const float* srcC, float* dst, size_t len) { vDSP_vsma (srcA, 1, &srcB, srcC, 1, dst, 1, len); }

    static void clampLow (const float* src, float thresh,     float* dst, size_t len) { vDSP_vthr (src, 1, &thresh, dst, 1, len); }
    static void clamp    (const float* src, float l, float h, float* dst, size_t len) { vDSP_vclip (src, 1, &l, &h, dst, 1, len); }

    static void intToFloat (const int32_t* src,  float* dst, size_t len) { vDSP_vflt32 (src, 1, dst, 1, len); }
    static void intToFloat (const uint32_t* src, float* dst, size_t len) { vDSP_vfltu32 (src, 1, dst, 1, len); }

    static float max        (const float* src, size_t len) { float r; vDSP_maxv (src, 1, &r, len); return r; }
    static float min        (const float* src, size_t len) { float r; vDSP_minv (src, 1, &r, len); return r; }
    static float sum        (const float* src, size_t len) { float r; vDSP_sve (src, 1, &r, len); return r; }
    static float mean       (const float* src, size_t len) { float r; vDSP_meanv (src, 1, &r, len); return r; }
    static float meanSquare (const float* src, size_t len) { float r; vDSP_measqv (src, 1, &r, len); return r; }
    static float rms        (const float* src, size_t len) { float r; vDSP_rmsqv (src, 1, &r, len); return r; }
    // clang-format on
};

template <>
class AppleAccelerate<double>
{
public:
    //==============================================================================
    // vForce functions
    //==============================================================================
    // clang-format off
    static void abs   (const double* src, double* dst, int len) { vvfabs (dst, src, &len); }
    static void ln    (const double* src, double* dst, int len) { vvlog (dst, src, &len); }
    static void log10 (const double* src, double* dst, int len) { vvlog10 (dst, src, &len); }
    static void log2  (const double* src, double* dst, int len) { vvlog2 (dst, src, &len); }
    static void exp   (const double* src, double* dst, int len) { vvexp (dst, src, &len); }
    static void exp2  (const double* src, double* dst, int len) { vvexp2 (dst, src, &len); }
    // clang-format on

    //==============================================================================
    // vDSP functions
    //==============================================================================
    // clang-format off
    static void abs (const double* src,                             double* dst, size_t len) { vDSP_vabsD (src, 1, dst, 1, len); }
    static void add (const double* srcA,  const double* srcB,       double* dst, size_t len) { vDSP_vaddD (srcA, 1, srcB, 1, dst, 1, len); }
    static void add (const double* srcA,  double srcB,              double* dst, size_t len) { vDSP_vsaddD (srcA, 1, &srcB, dst, 1, len); }
    static void sub (const double* srcA,  const double* srcB,       double* dst, size_t len) { vDSP_vsubD (srcB, 1, srcA, 1, dst, 1, len); }
    static void mul (const double* srcA,  const double* srcB,       double* dst, size_t len) { vDSP_vmulD (srcA, 1, srcB, 1, dst, 1, len); }
    static void mul (const double* srcA,  double srcB,              double* dst, size_t len) { vDSP_vsmulD (srcA, 1, &srcB, dst, 1, len); }
    static void div (const double* srcA,  const double* srcB,       double* dst, size_t len) { vDSP_vdivD (srcB, 1, srcA, 1, dst, 1, len); }
    static void div (const double* srcA,  double srcB,              double* dst, size_t len) { vDSP_vsdivD (srcA, 1, &srcB, dst, 1, len); }

    static void multiplyAdd (const double* srcA, double srcB, double srcC, double* dst, size_t len) { vDSP_vsmsaD (srcA, 1, &srcB, &srcC, dst, 1, len); }
    static void multiplyAdd (const double* srcA, const double* srcB, const double* srcC, double* dst, size_t len) { vDSP_vmaD (srcA, 1, srcB, 1, srcC, 1, dst, 1, len); }
    static void multiplyAdd (const double* srcA, double srcB,        const double* srcC, double* dst, size_t len) { vDSP_vsmaD (srcA, 1, &srcB, srcC, 1, dst, 1, len); }
    // clang-format on

    static void clampLow (const double* src, double thresh,      double* dst, size_t len) { vDSP_vthrD (src, 1, &thresh, dst, 1, len); }
    static void clamp    (const double* src, double l, double h, double* dst, size_t len) { vDSP_vclipD (src, 1, &l, &h, dst, 1, len); }

    static void intToFloat (const int32_t* src, double* dst, size_t len)
    {
        VCTR_ASSERT ((void*) src != (void*) dst);
        vDSP_vflt32D (src, 1, dst, 1, len);
    }

    static void intToFloat (const uint32_t* src, double* dst, size_t len)
    {
        VCTR_ASSERT ((void*) src != (void*) dst);
        vDSP_vfltu32D (src, 1, dst, 1, len);
    }

    static double max        (const double* src, size_t len) { double r; vDSP_maxvD (src, 1, &r, len); return r; }
    static double min        (const double* src, size_t len) { double r; vDSP_minvD (src, 1, &r, len); return r; }
    static double sum        (const double* src, size_t len) { double r; vDSP_sveD (src, 1, &r, len); return r; }
    static double mean       (const double* src, size_t len) { double r; vDSP_meanvD (src, 1, &r, len); return r; }
    static double meanSquare (const double* src, size_t len) { double r; vDSP_measqvD (src, 1, &r, len); return r; }
    static double rms        (const double* src, size_t len) { double r; vDSP_rmsqvD (src, 1, &r, len); return r; }
};

template <>
class AppleAccelerate<std::complex<float>>
{
private:
    // clang-format off
    static auto       sp (std::complex<float>* c)        { return DSPSplitComplex { reinterpret_cast<float*> (c), reinterpret_cast<float*> (c) + 1 }; }
    static const auto sp (const std::complex<float>* c)  { return sp (const_cast<std::complex<float>*> (c)); }
    static auto       sp (std::complex<float>& c)        { return DSPSplitComplex { &reinterpret_cast<float&> (c), &reinterpret_cast<float&> (c) + 1 }; }
    static const auto sp (const std::complex<float>& c)  { return sp (const_cast<std::complex<float>&> (c)); }
    // clang-format on

public:
    //==============================================================================
    // vDSP functions
    //==============================================================================
    static void abs (const std::complex<float>* src, float* dst, size_t len)
    {
        auto s = sp (src);
        vDSP_zvabs (&s, 2, dst, 1, len);
    }

    static void add (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_zvadd (&sa, 2, &sb, 2, &d, 2, len);
    }

    static void add (const std::complex<float>* srcA, std::complex<float> srcB, std::complex<float>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_vsadd (sa.realp, 2, sb.realp, d.realp, 2, len);
        vDSP_vsadd (sa.imagp, 2, sb.imagp, d.imagp, 2, len);
    }

    static void sub (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_zvsub (&sa, 2, &sb, 2, &d, 2, len);
    }

    static void mul (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_zvmul (&sa, 2, &sb, 2, &d, 2, len, 1);
    }

    static void div (const std::complex<float>* srcA, const std::complex<float>* srcB, std::complex<float>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_zvdiv (&sb, 2, &sa, 2, &d, 2, len);
    }
};

template <>
class AppleAccelerate<std::complex<double>>
{
private:
    // clang-format off
    static auto       sp (std::complex<double>* c)        { return DSPDoubleSplitComplex { reinterpret_cast<double*> (c), reinterpret_cast<double*> (c) + 1}; }
    static const auto sp (const std::complex<double>* c)  { return sp (const_cast<std::complex<double>*> (c)); }
    static auto       sp (std::complex<double>& c)        { return DSPDoubleSplitComplex { &reinterpret_cast<double&> (c), &reinterpret_cast<double&> (c) + 1 }; }
    static const auto sp (const std::complex<double>& c)  { return sp (const_cast<std::complex<double>&> (c)); }
    // clang-format on

public:
    //==============================================================================
    // vDSP functions
    //==============================================================================
    static void abs (const std::complex<double>* src, double* dst, size_t len)
    {
        auto sc = sp (src);
        vDSP_zvabsD (&sc, 2, dst, 1, len);
    }

    static void add (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_zvaddD (&sa, 2, &sb, 2, &d, 2, len);
    }

    static void add (const std::complex<double>* srcA, std::complex<double> srcB, std::complex<double>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_vsaddD (sa.realp, 2, sb.realp, d.realp, 2, len);
        vDSP_vsaddD (sa.imagp, 2, sb.imagp, d.imagp, 2, len);
    }

    static void sub (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_zvsubD (&sa, 2, &sb, 2, &d, 2, len);
    }

    static void mul (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_zvmulD (&sa, 2, &sb, 2, &d, 2, len, 1);
    }

    static void div (const std::complex<double>* srcA, const std::complex<double>* srcB, std::complex<double>* dst, size_t len)
    {
        auto sa = sp (srcA);
        auto sb = sp (srcB);
        auto d = sp (dst);
        vDSP_zvdivD (&sb, 2, &sa, 2, &d, 2, len);
    }
};

#endif
} // namespace vctr::PlatformVectorOps
