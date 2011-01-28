#ifndef __DCDLOADER__H__
#define __DCDLOADER__H__

#include <boost/python.hpp>
#include <boost/shared_array.hpp>

#include "molfile/molfile_plugins.h"
#include "num_util.h"
#include "trajectory.h"

#include <cassert>
#include <string>

//! Class for loading DCD files into freud
/*! The structure information is assumed to have been read in elsewhere.

	Every call to readNextStep() will fill out the existing Nx3 numpy array. Users in python will have to be aware
    of this and plan accordingly (specifically TrajectoryXMLDCD will make a copy so that users aren't confused)
    
    The values read in can be accessed with getPoints() and getBox()
    
    jumpToFrame() is smart in that it can be called as many times as needed and it will not re-read the file
    if it only has to advance. If a previous frame is selected, however, jumpToFrame has no choice but to close
    and reopen the file and reread it from the beginning.
	*/
class DCDLoader
	{
	public:
		//! Constructs the loader and associates it to the given file
		DCDLoader(const std::string &dcd_fname);
		//! Frees all dynamic memory
		~DCDLoader();
		
		//! Jumps to a particular frame number in the file
		void jumpToFrame(int frame);
        
		//! Read the next step in the file
		void readNextFrame();
		
        //! Access the points read by the last step
        boost::python::numeric::array getPoints() const
            {
            return m_points;
            }
        
        //! Get the box
        const Box& getBox() const
            {
            return m_box;
            }
        
        //! Get the number of particles
        unsigned int getNumParticles() const
            {
            assert(m_dcd);
            return m_dcd->natoms;
            }
        
        //! Get the last frame read
        unsigned int getLastFrameNum() const
            {
            assert(m_dcd);
            // the frame counter is advanced when read, so subtract 1 to get the last one read
            return m_dcd->setsread-1;
            }
        
        //! Get the number of frames
        unsigned int getFrameCount() const
            {
            assert(m_dcd);
            return m_dcd->nsets;
            }
        
	private:
        std::string m_fname;                        //!< File name of the DCD file
        Box m_box;                                  //!< The box read from the last readNextStep()
        boost::python::numeric::array m_points;     //!< Points read during the last readNextStep()
        
		//! Keep track of the dcd file
		dcdhandle *m_dcd;
		
		//! Keep track of the dcd plugin
		molfile_plugin_t *dcdplugin;
		
		//! Helper function to start loading the dcd file
		void loadDCD();
    };
	
void export_dcdloader();
#endif