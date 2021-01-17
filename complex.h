#pragma once

#include <cmath>

class complex
{
    double _re, _im;

public:
    // Complex zero (default constructor)
    complex()
    {
        _re = .0;
        _im = .0;
    }

    // Complex number constructor
    complex( double re, double im )
    {
        _re = re;
        _im = im;
    }

    // Real and Imaginary parts
    double re() const
    {
        return _re;
    }
    double im() const
    {
        return _im;
    }

    // Complex number addition
    complex operator+( const complex & c ) const
    {
        return complex( _re + c._re, _im + c._im );
    }

    // Complex number multiplication
    complex operator*( const complex & c ) const
    {
        return complex( _re * c._re - _im * c._im, _re * c._im + _im * c._re );
    }

    // Absolute value
    double abs() const
    {
        return std::sqrt( _re * _re + _im * _im );
    }
};
