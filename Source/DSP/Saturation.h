#pragma once
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
//  ADAA — Antiderivative Anti-Aliasing di primo ordine per tanh()
//
//  Un waveshaper genera armoniche molto oltre Nyquist. In tempo discreto
//  quelle armoniche non spariscono: si ripiegano dentro la banda audio a
//  frequenze non armoniche. E' esattamente il timbro metallico e stridulo che
//  si sente sulle note alte e con il drive aperto — non e' saturazione, e'
//  aliasing.
//
//  La soluzione ovvia sarebbe oversamplare, ma qui la catena e' campione per
//  campione e infilare un oversampler significherebbe riscriverla. ADAA
//  risolve lo stesso problema restando per-campione: invece di valutare la
//  curva nel punto, la si integra sul segmento percorso tra due campioni
//
//      y[n] = ( F(x[n]) - F(x[n-1]) ) / ( x[n] - x[n-1] )        F' = f
//
//  cioe' il valore medio della nonlinearita' lungo il tratto. Le transizioni
//  brusche vengono smussate prima che possano generare le armoniche alte, e
//  l'aliasing cala di parecchi dB al costo di una memoria di un campione.
//
//  Rif. Parker, Zavalishin, Le Bivic, "Reducing the Aliasing of Nonlinear
//  Waveshaping Using Continuous-Time Convolution", DAFx-16.
// ─────────────────────────────────────────────────────────────────────────────
class AdaaTanh
{
public:
    void reset()
    {
        xPrev = 0.0;
        fPrev = 0.0;   // antiderivative(0) == 0
    }

    double process (double x)
    {
        const double F  = antiderivative (x);
        const double dx = x - xPrev;

        // Quando i due campioni quasi coincidono il rapporto e' 0/0: sotto
        // soglia si ricade sulla valutazione diretta nel punto medio, che li'
        // e' indistinguibile dal limite ed e' numericamente stabile.
        const double y = (std::abs (dx) < 1.0e-5)
                       ? std::tanh (0.5 * (x + xPrev))
                       : (F - fPrev) / dx;

        xPrev = x;
        fPrev = F;
        return y;
    }

private:
    // Primitiva di tanh: ln(cosh(x)).
    // Scritta come |x| + ln(1+e^-2|x|) - ln2 perche' cosh(x) va in overflow
    // gia' intorno a x=710, e con il drive a fondo corsa ci si arriva.
    static double antiderivative (double x)
    {
        const double a = std::abs (x);
        return a + std::log1p (std::exp (-2.0 * a)) - 0.69314718055994531;
    }

    double xPrev = 0.0;
    double fPrev = 0.0;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Filtro a un polo, usato come damping e come tone shaping.
//  Volutamente minimale: nel percorso di feedback del delay serve dolcezza,
//  non pendenza.
// ─────────────────────────────────────────────────────────────────────────────
class OnePoleLP
{
public:
    void prepare (double sampleRate, double cutoffHz)
    {
        sr = sampleRate;
        setCutoff (cutoffHz);
        reset();
    }

    void setCutoff (double cutoffHz)
    {
        const double fc = (cutoffHz < 10.0)      ? 10.0
                        : (cutoffHz > sr * 0.45) ? sr * 0.45
                                                 : cutoffHz;
        coeff = 1.0 - std::exp (-2.0 * 3.14159265358979323846 * fc / sr);
    }

    void  reset()            { z = 0.0; }
    float process (float x)
    {
        z += coeff * (static_cast<double> (x) - z);
        return static_cast<float> (z);
    }

private:
    double sr    = 44100.0;
    double coeff = 1.0;
    double z     = 0.0;
};
