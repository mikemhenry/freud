#include "pairing.h"

#include <stdexcept>
#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef ENABLE_OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace boost::python;

namespace freud { namespace pairing {

pairing::pairing(const trajectory::Box& box, float rmax,
                        float shape_dot_target, float shape_dot_tol, float comp_dot_target, float comp_dot_tol)
    : m_box(box), m_rmax(rmax), m_shape_dot_target(shape_dot_target), m_shape_dot_tol(shape_dot_tol),
    m_comp_dot_target(comp_dot_target), m_comp_dot_tol(comp_dot_tol)
    {
    if (rmax < 0.0f)
        throw invalid_argument("rmax must be positive");
    if (rmax > box.getLx()/2 || rmax > box.getLy()/2)
    throw invalid_argument("rmax must be smaller than half the smallest box size");

    if (useCells())
    {
    m_lc = new locality::LinkCell(box, rmax);
    }
    }

pairing::~pairing()
    {
    if(useCells())
    delete m_lc;
    }

bool pairing::useCells()
    {
    float l_min = fmin(m_box.getLx(), m_box.getLy());
    if (m_box.is2D())
    l_min = fmin(l_min, m_box.getLy());
    if (m_rmax < l_min/3)
    return true;
    return false;
    }

// Need to cite this
// bool pairing::sameSide(float3 A, float3 B, float3 r, float3 p)
//     {
//     float3 BA;
//     float3 rA;
//     float3 pA;

//     BA.x = B.x - A.x;
//     BA.y = B.y - A.y;
//     BA.z = B.z - A.z;

//     rA.x = r.x - A.x;
//     rA.y = r.y - A.y;
//     rA.z = r.z - A.z;

//     pA.x = p.x - A.x;
//     pA.y = p.y - A.y;
//     pA.z = p.z - A.z;

//     float3 ref = cross(BA, rA);
//     float3 test = cross(BA, pA);
//     if (dot3(ref, test) >= 0)
//         {
//         return true;
//         }
//     else
//         {
//         return false;
//         }
//     }

// bool pairing::isInside(float2 t[], float2 p)
//     {
//     float3 nt [3];
//     float3 np;

//     for (unsigned int i = 0; i < 3; i++)
//         {
//         nt[i].x = t[i].x;
//         nt[i].y = t[i].y;
//         nt[i].z = 0;
//         }

//     np.x = p.x;
//     np.y = p.y;
//     np.z = 0;

//     return isInside(nt, np);

//     }

// bool pairing::isInside(float3 t[], float3 p)
//     {
//     float3 A;
//     float3 B;
//     float3 C;
//     float3 P;

//     // Even though float threes are taken in, the z component is assumed zero
//     // i.e. all in the same plane

//     A.x = t[0].x;
//     A.y = t[0].y;
//     A.z = 0;

//     B.x = t[1].x;
//     B.y = t[1].y;
//     B.z = 0;

//     C.x = t[2].x;
//     C.y = t[2].y;
//     C.z = 0;

//     P.x = p.x;
//     P.y = p.y;
//     P.z = 0;

//     bool BC = sameSide(B, C, A, P);
//     bool AC = sameSide(A, C, B, P);
//     bool AB = sameSide(A, B, C, P);

//     if (AB && BC && AC)
//         {
//         return true;
//         }
//     else
//         {
//         return false;
//         }

//     }

// float2 pairing::mat_rotate(float2 point, float angle)
//     {
//     float2 rot;
//     float mysin = sinf(angle);
//     float mycos = cosf(angle);
//     rot.x = mycos * point.x - mysin * point.y;
//     rot.y = mysin * point.x + mycos * point.y;
//     return rot;
//     }

// float2 pairing::into_local(float2 ref_point,
//                             float2 point,
//                             float2 vert,
//                             float ref_angle,
//                             float angle)
//     {
//     float2 local;
//     local = mat_rotate(mat_rotate(vert, -ref_angle), angle);
//     float2 vec;
//     vec.x = point.x - ref_point.x;
//     vec.y = point.y - ref_point.y;
//     vec = mat_rotate(vec, -ref_angle);
//     local.x = local.x + vec.x;
//     local.y = local.y + vec.y;
//     return local;
//     }

// float pairing::cavity_depth(float2 t[])
//     {
//     float2 v_mouth;
//     float2 v_side;

//     v_mouth.x = t[0].x - t[2].x;
//     v_mouth.y = t[0].y - t[2].y;
//     float m_mouth = sqrt(dot2(v_mouth, v_mouth));
//     v_side.x = t[1].x - t[2].x;
//     v_side.y = t[1].y - t[2].y;

//     float3 a_vec = cross(v_mouth, v_side);
//     float area = sqrt(dot3(a_vec, a_vec));
//     return area/m_mouth;
//     }

// multiply two quaternions together
// float4 pairing::quat_mult(float4 a, float4 b)
//     {
//     float4 c;
//     c.w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z);
//     c.x = (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y);
//     c.y = (a.w * b.y) - (a.x * b.z) + (a.y * b.w) + (a.z * b.x);
//     c.z = (a.w * b.z) + (a.x * b.y) - (a.y * b.x) + (a.z * b.w);
//     return c;
//     }
//
// // generate a quaternion from a vector
// float4 gen_q(angle):
//     axis = numpy.array([0, 0, 1], dtype=numpy.float32)
//     q = numpy.zeros(shape=(4), dtype=numpy.float32)
//     q[0] = numpy.cos(0.5 * angle)
//     q[1] = axis[0] * numpy.sin(0.5 * angle)
//     q[2] = axis[1] * numpy.sin(0.5 * angle)
//     q[3] = axis[2] * numpy.sin(0.5 * angle)
//     den = numpy.sqrt(numpy.dot(q, q))
//     q /= den
//     return q
//
// float4 gen_qs(angle):
//     axis = numpy.array([0, 0, 1], dtype=numpy.float32)
//     q = numpy.zeros(shape=(4), dtype=numpy.float32)
//     q[0] = numpy.cos(0.5 * angle)
//     q[1] = -axis[0] * numpy.sin(0.5 * angle)
//     q[2] = -axis[1] * numpy.sin(0.5 * angle)
//     q[3] = -axis[2] * numpy.sin(0.5 * angle)
//     den = numpy.sqrt(numpy.dot(q, q))
//     q /= den
//     return q
//
// float4 q_rotate(quat, angle):
//     q = gen_q(angle)
//     qs = gen_qs(angle)
//     tp = numpy.zeros(shape=(4), dtype=numpy.float32)
//     tp = quat
//     tmp = quat_mult(q, tp)
//     ps = quat_mult(tmp, qs)
//     return ps
//
// float calc_angle(float3 v1, float3 v2)
//     {
//     // find axis of rotation
//     // need to fins vector
//     float3 axis = cross(v1, v2);
//
//     }
//
// def gen_quats(v1, v2):
//     axis = numpy.cross(v1, v2);
//     #axis = numpy.array([0.0, 1.0, 0.0])
//     axis = axis / numpy.sqrt(numpy.dot(axis, axis));
//     angle = numpy.arccos(numpy.dot(v1, v2)/(numpy.sqrt(numpy.dot(v1, v1)) * numpy.sqrt(numpy.dot(v2, v2))));
//     #angle = numpy.pi/4;
//     q = numpy.array([numpy.cos(0.5 * angle), axis[0] * numpy.sin(0.5 * angle), axis[1] * numpy.sin(0.5 * angle), axis[2] * numpy.sin(0.5 * angle)]);
//     qs = numpy.array([numpy.cos(0.5 * angle), -1.0 * axis[0] * numpy.sin(0.5 * angle), -1.0 * axis[1] * numpy.sin(0.5 * angle), -1.0 * axis[2] * numpy.sin(0.5 * angle)]);
//     q = q / numpy.sqrt(numpy.dot(q,q))
//     qs = qs / numpy.sqrt(numpy.dot(qs,qs))
//     print("axis = " + str(axis) + "\n");
//     print("angle = " + str(angle) + "\n");
//     print("q = " + str(q) + "\n");
//     print("qs = " + str(qs) + "\n");
//
float3 pairing::cross(float2 v1, float2 v2)
    {
    float3 v1_n;
    float3 v2_n;
    v1_n.x = v1.x;
    v1_n.y = v1.y;
    v1_n.z = 0;
    v2_n.x = v2.x;
    v2_n.y = v2.y;
    v2_n.z = 0;
    return cross(v1_n, v2_n);
    }

float3 pairing::cross(float3 v1, float3 v2)
    {
    float3 v;
    v.x = (v1.y * v2.z) - (v2.y * v1.z);
    v.y = (v2.x * v1.z) - (v1.x * v2.z);
    v.z = (v1.x * v2.y) - (v2.x * v1.y);
    return v;
    }

float pairing::dot2(float2 v1, float2 v2)
    {
    return (v1.x * v2.x) + (v1.y * v2.y);
    }

float pairing::dot3(float3 v1, float3 v2)
    {
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
    }

bool pairing::comp_check(float3 r_i,
                            float3 r_j,
                            float angle_s_i,
                            float angle_s_j,
                            float angle_c_i,
                            float angle_c_j)
    {
    float rmaxsq = m_rmax * m_rmax;
    // calculate the vector from shape i to shape j
    float2 r_ij;
    float2 r_ij_u;
    float2 r_ji_u;
    // r_ij.x = r_j.x - r_i.x;
    // r_ij.y = r_j.y - r_i.y;
    // allow for periodic conditions
    float dx = r_j.x - r_i.x;
    float dy = r_j.y - r_i.y;
    float dz = (float) 0.0;
    float3 delta = m_box.wrap(make_float3(dx, dy, dz));
    r_ij.x = delta.x;
    r_ij.y = delta.y;
    float r_ij_mag = sqrt(dot2(r_ij, r_ij));
    r_ij_u.x = r_ij.x / r_ij_mag;
    r_ij_u.y = r_ij.y / r_ij_mag;
    r_ji_u.x = -r_ij_u.x;
    r_ji_u.y = -r_ij_u.y;
    // calculate the orientation vectors for shapes i and j
    float2 theta_s_i;
    theta_s_i.x = cos(angle_s_i);
    theta_s_i.y = sin(angle_s_i);
    float2 theta_s_j;
    theta_s_j.x = cos(angle_s_j);
    theta_s_j.y = sin(angle_s_j);
    float2 theta_c_i;
    theta_c_i.x = cos(angle_c_i);
    theta_c_i.y = sin(angle_c_i);
    float2 theta_c_j;
    theta_c_j.x = cos(angle_c_j);
    theta_c_j.y = sin(angle_c_j);
    // find the square of the distance between the particles
    float d_ij = dot2(r_ij, r_ij);
    // find the dot products of the orientation vectors
    float theta_s_ij = dot2(theta_s_i, theta_s_j);
    float theta_c_ij = dot2(theta_c_i, theta_c_j);
    // find the dot product of the interparticle vector and shape i's comp orientation
    float v_ij = dot2(r_ij_u, theta_c_i);
    float v_ji = dot2(r_ji_u, theta_c_j);
    // determine if paired
    if (d_ij > rmaxsq)
        return false;
    if ((theta_s_ij < (m_shape_dot_target - m_shape_dot_tol)) || (theta_s_ij > (m_shape_dot_target + m_shape_dot_tol)))
        return false;
    if ((v_ij < (m_comp_dot_target - m_comp_dot_tol)) || (v_ij > (m_comp_dot_target + m_comp_dot_tol)))
        return false;
    if ((v_ji < (m_comp_dot_target - m_comp_dot_tol)) || (v_ji > (m_comp_dot_target + m_comp_dot_tol)))
        return false;
    return true;
    }

void pairing::compute(unsigned int* match,
                float3* points,
                float* shape_angles,
                float* comp_angles,
                unsigned int Np)
    {
    m_nmatch = 0;
    if (useCells())
        {
        computeWithCellList(match,
                            points,
                            shape_angles,
                            comp_angles,
                            Np);
        }
    else
        {
        computeWithoutCellList(match,
                            points,
                            shape_angles,
                            comp_angles,
                            Np);
        }
    }

void pairing::computeWithoutCellList(unsigned int* match,
                float3* points,
                float* shape_angles,
                float* comp_angles,
                unsigned int Np)
    {
    m_nP = Np;
    #pragma omp parallel
        {
        #pragma omp for schedule(guided)
        // for each reference point
        for (unsigned int i = 0; i < m_nP; i++)
            {
            // grab point and type
            // might be eliminated later for efficiency
            float3 r_i = points[i];
            float angle_s_i = shape_angles[i];
            float angle_c_i = comp_angles[i];
            for (unsigned int j = 0; j < m_nP; j++)
                {
                float3 r_j = points[j];
                float angle_s_j = shape_angles[j];
                float angle_c_j = comp_angles[j];
                // will skip same particle
                if (i == j)
                    {
                    continue;
                    }

                if (comp_check(r_i,
                            r_j,
                            angle_s_i,
                            angle_s_j,
                            angle_c_i,
                            angle_c_j))
                    {
                    match[i] = 1;
                    match[j] = 1;
                    m_nmatch++;
                    }
                } // done looping over check
            } //done looping over ref
        } // End of parallel section
    }

void pairing::computeWithCellList(unsigned int* match,
                float3* points,
                float* shape_angles,
                float* comp_angles,
                unsigned int Np)
    {
    m_nP = Np;
    m_lc->computeCellList(points, m_nP);
    #pragma omp parallel
        {
        #pragma omp for schedule(guided)
        // for each particle
        for (unsigned int i = 0; i < m_nP; i++)
            {
            // grab point and type
            // might be eliminated later for efficiency
            float3 r_i = points[i];
            float angle_s_i = shape_angles[i];
            float angle_c_i = comp_angles[i];
            // get the cell the point is in
            unsigned int ref_cell = m_lc->getCell(r_i);
            // loop over all neighboring cells
            const std::vector<unsigned int>& neigh_cells = m_lc->getCellNeighbors(ref_cell);
            for (unsigned int neigh_idx = 0; neigh_idx < neigh_cells.size(); neigh_idx++)
                {
                unsigned int neigh_cell = neigh_cells[neigh_idx];
                // iterate over the particles in that cell
                locality::LinkCell::iteratorcell it = m_lc->itercell(neigh_cell);
                for (unsigned int j = it.next(); !it.atEnd(); j=it.next())
                    {
                    float3 r_j = points[j];
                    float angle_s_j = shape_angles[j];
                    float angle_c_j = comp_angles[j];
                    // will skip same particle
                    if (i == j)
                        {
                        continue;
                        }

                    if (comp_check(r_i,
                            r_j,
                            angle_s_i,
                            angle_s_j,
                            angle_c_i,
                            angle_c_j))
                        {
                        match[i] = 1;
                        match[j] = 1;
                        m_nmatch++;
                        }
                    } // done looping over neighbors
                } // done looping over neighbor cells
            } // done looping over reference points
        } // End of parallel section
    }

void pairing::computePy(boost::python::numeric::array match,
                    boost::python::numeric::array points,
                    boost::python::numeric::array shape_angles,
                    boost::python::numeric::array comp_angles)
    {
    // points contains all the particle positions; Np x 3
    // types contains all the types; Np (x 1)
    // angles contains the angle of each particle; Np (x1)
    // shapes contains the verticies of each type; Nt x Nv_max
    //      where any unpop'd vertices are nan
    // ref_list contains the types of shapes that will be referenced
    // check_list contains the types of shapes that will be checked
    // ref_verts contains the vert index that will be checked
    // check_verts contains the vert index that will be checked
    // match will contain the particle index of the ref that is matched
    num_util::check_type(match, PyArray_INT);
    num_util::check_rank(match, 1);
    num_util::check_type(points, PyArray_FLOAT);
    num_util::check_rank(points, 2);
    num_util::check_type(shape_angles, PyArray_FLOAT);
    num_util::check_rank(shape_angles, 1);
    num_util::check_type(comp_angles, PyArray_FLOAT);
    num_util::check_rank(comp_angles, 1);

    // get the number of particles
    // validate that the 2nd dimension is only 3
    num_util::check_dim(points, 1, 3);
    unsigned int Np = num_util::shape(points)[0];

    //validate that the types and angles coming in are the correct size
    num_util::check_dim(shape_angles, 0, Np);
    num_util::check_dim(comp_angles, 0, Np);

    // get the raw data pointers and compute the cell list
    unsigned int* match_raw = (unsigned int*) num_util::data(match);
    float3* points_raw = (float3*) num_util::data(points);
    float* shape_angles_raw = (float*) num_util::data(shape_angles);
    float* comp_angles_raw = (float*) num_util::data(comp_angles);

    compute(match_raw,
            points_raw,
            shape_angles_raw,
            comp_angles_raw,
            Np);
    }

void export_pairing()
    {
    class_<pairing>("pairing", init<trajectory::Box&, float, float, float, float, float>())
        .def("getBox", &pairing::getBox, return_internal_reference<>())
        .def("compute", &pairing::computePy)
        .def("getNpair", &pairing::getNpairPy)
        ;
    }

}; }; // end namespace freud::pairing
