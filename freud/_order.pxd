# Copyright (c) 2010-2018 The Regents of the University of Michigan
# This file is from the freud project, released under the BSD 3-Clause License.

from libcpp cimport bool
from freud.util._VectorMath cimport vec3
from freud.util._VectorMath cimport quat
from libcpp.memory cimport shared_ptr
from libcpp.complex cimport complex
from libcpp.vector cimport vector
from libcpp.map cimport map
cimport freud._box as box
cimport freud._locality

cdef extern from "CubaticOrderParameter.h" namespace "freud::order":
    cdef cppclass CubaticOrderParameter:
        CubaticOrderParameter(float,
                              float,
                              float,
                              float*,
                              unsigned int,
                              unsigned int)
        void reset()
        void compute(quat[float]*,
                     unsigned int,
                     unsigned int) nogil except +
        void reduceCubaticOrderParameter()
        unsigned int getNumParticles()
        float getCubaticOrderParameter()
        shared_ptr[float] getParticleCubaticOrderParameter()
        shared_ptr[float] getParticleTensor()
        shared_ptr[float] getGlobalTensor()
        shared_ptr[float] getCubaticTensor()
        shared_ptr[float] getGenR4Tensor()
        float getTInitial()
        float getTFinal()
        float getScale()
        quat[float] getCubaticOrientation()

cdef extern from "NematicOrderParameter.h" namespace "freud::order":
    cdef cppclass NematicOrderParameter:
        NematicOrderParameter(vec3[float])
        void reset()
        void compute(quat[float]*,
                     unsigned int) nogil except +
        unsigned int getNumParticles()
        float getNematicOrderParameter()
        shared_ptr[float] getParticleTensor()
        shared_ptr[float] getNematicTensor()
        vec3[float] getNematicDirector()


cdef extern from "HexOrderParameter.h" namespace "freud::order":
    cdef cppclass HexOrderParameter:
        HexOrderParameter(float, unsigned int, unsigned int)
        const box.Box & getBox() const
        void compute(box.Box &,
                     const freud._locality.NeighborList*,
                     const vec3[float]*,
                     unsigned int) nogil except +
        # unsure how to pass back the std::complex,
        # but this seems to compile...
        shared_ptr[float complex] getPsi()
        unsigned int getNP()
        unsigned int getK()

cdef extern from "TransOrderParameter.h" namespace "freud::order":
    cdef cppclass TransOrderParameter:
        TransOrderParameter(float, float)
        const box.Box & getBox() const,
        void compute(box.Box &,
                     const freud._locality.NeighborList*,
                     const vec3[float]*,
                     unsigned int) nogil except +
        shared_ptr[float complex] getDr()
        unsigned int getNP()

cdef extern from "LocalQl.h" namespace "freud::order":
    cdef cppclass LocalQl:
        LocalQl(const box.Box &, float, unsigned int, float)
        const box.Box & getBox() const
        unsigned int getNP()
        void setBox(const box.Box)
        shared_ptr[float] getQl()

        void compute(const freud._locality.NeighborList *,
                     const vec3[float]*,
                     unsigned int) nogil except +
        void computeAve(const freud._locality.NeighborList *,
                        const vec3[float]*,
                        unsigned int) nogil except +
        void computeNorm(const vec3[float]*,
                         unsigned int) nogil except +
        void computeAveNorm(const vec3[float]*,
                            unsigned int) nogil except +
        shared_ptr[float] getAveQl()
        shared_ptr[float] getQlNorm()
        shared_ptr[float] getQlAveNorm()

cdef extern from "LocalWl.h" namespace "freud::order":
    cdef cppclass LocalWl(LocalQl):
        LocalWl(const box.Box &, float, unsigned int, float)
        shared_ptr[float complex] getWl()
        shared_ptr[float complex] getAveWl()
        shared_ptr[float complex] getWlNorm()
        shared_ptr[float complex] getAveNormWl()
        void enableNormalization()
        void disableNormalization()

cdef extern from "SolLiq.h" namespace "freud::order":
    cdef cppclass SolLiq:
        SolLiq(const box.Box &, float, float, unsigned int, unsigned int)
        const box.Box & getBox() const
        void setBox(const box.Box)
        void setClusteringRadius(float)
        void compute(const freud._locality.NeighborList *,
                     const vec3[float]*,
                     unsigned int) nogil except +
        void computeSolLiqVariant(const freud._locality.NeighborList *,
                                  const vec3[float]*,
                                  unsigned int) nogil except +
        void computeSolLiqNoNorm(const freud._locality.NeighborList *,
                                 const vec3[float]*,
                                 unsigned int) nogil except +
        unsigned int getLargestClusterSize()
        vector[unsigned int] getClusterSizes()
        shared_ptr[float complex] getQlmi()
        shared_ptr[unsigned int] getClusters()
        shared_ptr[unsigned int] getNumberOfConnections()
        vector[float complex] getQldot_ij()
        unsigned int getNP()
        unsigned int getNumClusters()
