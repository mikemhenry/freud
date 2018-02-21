// Copyright (c) 2010-2018 The Regents of the University of Michigan
// This file is part of the freud project, released under the BSD 3-Clause License.

#include <tbb/tbb.h>
#include <ostream>

// work around nasty issue where python #defines isalpha, toupper, etc....
#undef __APPLE__
#include <Python.h>
#define __APPLE__

#include <memory>

#include "HOOMDMath.h"
#include "VectorMath.h"

#include "LinkCell.h"
#include "box.h"
#include "Index1D.h"

#ifndef _PMFTXYZ_H__
#define _PMFTXYZ_H__

/*! \internal
    \file PMFT.h
    \brief Declares base class for all PMFT classes
*/

namespace freud { namespace pmft {

//! Computes the PMFT for a given set of points
/*! The PMFT class is an abstract class providing the basis for all classes calculating PMFTs for specific
 *  dimensional cases. The PMFT class defines some of the key interfaces required for all PMFT classes, such
 *  as the ability to access the underlying PCF and box. Many of the specific methods must be implemented by
 *  subclasses that account for the proper set of dimensions.
*/
class PMFTXYZ
    {
    public:
        //! Constructor
        PMFT();

        //! Destructor
        ~PMFT();

        //! Get the simulation box
        const box::Box& getBox() const
            {
            return m_box;
            }

        //! Reset the PCF array to all zeros
        void resetPCF();

        //! \internal
        //! helper function to reduce the thread specific arrays into one array
        void reducePCF();

        //! Get a reference to the PCF array
        std::shared_ptr<float> getPCF();

        //! Get a reference to the bin counts array
        std::shared_ptr<unsigned int> getBinCounts();

        float getRCut()
            {
            return m_r_cut;
            }

    private:
        box::Box m_box;                    //!< Simulation box where the particles belong
        float m_r_cut;                     //!< r_cut used in cell list construction
        unsigned int m_frame_counter;      //!< number of frames calc'd
        unsigned int m_n_ref;              //!<TODO: Document this
        bool m_reduce;                     //!< Whether or not the PCF has been reduced yet

        std::shared_ptr<float> m_pcf_array;            //!< array of pcf computed
        std::shared_ptr<unsigned int> m_bin_counts;    //!< Counts for each bin (will differ for each PMFT)
        tbb::enumerable_thread_specific<unsigned int *> m_local_bin_counts; //!< TODO: Dcoument this
    };

}; }; // end namespace freud::pmft

#endif // _PMFT_H__
