// Copyright (c) 2010-2019 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#include <cassert>
#include <complex>
#include <stdexcept>
#include <tbb/tbb.h>
#ifdef __SSE2__
#include <emmintrin.h>
#endif

#include <iostream>

#include "CorrelationFunction.h"
#include "NeighborComputeFunctional.h"

using namespace std;
using namespace tbb;

/*! \file CorrelationFunction.cc
    \brief Generic pairwise correlation functions.
*/

namespace freud { namespace density {

template<typename T>
CorrelationFunction<T>::CorrelationFunction(float rmax, float dr)
    : m_box(box::Box()), m_rmax(rmax), m_dr(dr), m_frame_counter(0), m_reduce(true)
{
    if (dr <= 0.0f)
        throw invalid_argument("CorrelationFunction requires dr to be positive.");
    if (rmax <= 0.0f)
        throw invalid_argument("CorrelationFunction requires rmax to be positive.");
    if (dr > rmax)
        throw invalid_argument("CorrelationFunction requires dr must be less than or equal to rmax.");

    m_nbins = int(floorf(m_rmax / m_dr));
    assert(m_nbins > 0);
    m_rdf_array = std::shared_ptr<T>(new T[m_nbins], std::default_delete<T[]>());
    // Less efficient: initialize each bin sequentially using default ctor
    for (size_t i(0); i < m_nbins; ++i)
        m_rdf_array.get()[i] = T();
    m_bin_counts
        = std::shared_ptr<unsigned int>(new unsigned int[m_nbins], std::default_delete<unsigned int[]>());
    memset((void*) m_bin_counts.get(), 0, sizeof(unsigned int) * m_nbins);

    // precompute the bin center positions
    m_r_array = std::shared_ptr<float>(new float[m_nbins], std::default_delete<float[]>());
    for (unsigned int i = 0; i < m_nbins; i++)
    {
        float r = float(i) * m_dr;
        float nextr = float(i + 1) * m_dr;
        m_r_array.get()[i] = 2.0f / 3.0f * (nextr * nextr * nextr - r * r * r) / (nextr * nextr - r * r);
    }
    m_local_bin_counts.resize(m_nbins);
    m_local_rdf_array.resize(m_nbins);
}

//! \internal
//! helper function to reduce the thread specific arrays into one array
template<typename T> void CorrelationFunction<T>::reduceCorrelationFunction()
{
    memset((void*) m_bin_counts.get(), 0, sizeof(unsigned int) * m_nbins);
    for (size_t i(0); i < m_nbins; ++i)
        m_rdf_array.get()[i] = T();
    // now compute the rdf
    parallel_for(tbb::blocked_range<size_t>(0, m_nbins), [=](const blocked_range<size_t>& r) {
        for (size_t i = r.begin(); i != r.end(); i++)
        {
            for (util::ThreadStorage<unsigned int>::const_iterator local_bins = m_local_bin_counts.begin();
                 local_bins != m_local_bin_counts.end(); ++local_bins)
            {
                m_bin_counts.get()[i] += (*local_bins)[i];
            }
            for (typename util::ThreadStorage<T>::const_iterator local_rdf = m_local_rdf_array.begin();
                 local_rdf != m_local_rdf_array.end(); ++local_rdf)
            {
                m_rdf_array.get()[i] += (*local_rdf)[i];
            }
            if (m_bin_counts.get()[i])
            {
                m_rdf_array.get()[i] /= m_bin_counts.get()[i];
            }
        }
    });
}

//! Get a reference to the RDF array
template<typename T> std::shared_ptr<T> CorrelationFunction<T>::getRDF()
{
    if (m_reduce == true)
    {
        reduceCorrelationFunction();
    }
    m_reduce = false;
    return m_rdf_array;
}

//! \internal
/*! \brief Function to reset the PCF array if needed e.g. calculating between new particle types
 */
template<typename T> void CorrelationFunction<T>::reset()
{
    // zero the bin counts for totaling
    m_local_rdf_array.reset();
    m_local_bin_counts.reset();
    // reset the frame counter
    m_frame_counter = 0;
    m_reduce = true;
}

template<typename T>
void CorrelationFunction<T>::accumulate(const box::Box& box, const freud::locality::NeighborList* nlist,
                                        const freud::locality::NeighborQuery* nq, const T* ref_values,
                                        unsigned int n_ref, const vec3<float>* points, const T* point_values,
                                        unsigned int Np, freud::locality::QueryArgs qargs)
{
    m_box = box;
    float dr_inv = 1.0f / m_dr;
    freud::locality::loopOverNeighbors(nq, points, Np, qargs, nlist,
    [=](size_t i, size_t j, float dist, float weight)
        {
            // check that the particle is not checking itself, if it is the same list
            if(dist < m_rmax)
            {
                // float r = sqrtf(rsq);
                float r = dist;

                // bin that r
                float binr = r * dr_inv;
                // fast float to int conversion with truncation
                #ifdef __SSE2__
                unsigned int bin = _mm_cvtt_ss2si(_mm_load_ss(&binr));
                #else
                unsigned int bin = (unsigned int)(binr);
                #endif

                if (bin < m_nbins)
                {
                    ++m_local_bin_counts.local()[bin];
                    m_local_rdf_array.local()[bin] += ref_values[i] * point_values[j];
                }
            }
        }
    );
    m_frame_counter += 1;
    m_reduce = true;
}

template class CorrelationFunction<complex<double>>;
template class CorrelationFunction<double>;

}; }; // end namespace freud::density
