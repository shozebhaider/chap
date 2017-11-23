#include <cmath>
#include <vector>

#include <gromacs/math/vec.h>
#include <gromacs/utility/real.h> 

#include "io/molecular_path_obj_exporter.hpp"


/*
 *
 */
RegularVertexGrid::RegularVertexGrid(
        std::vector<real> s,
        std::vector<real> phi)
    : s_(s)
    , phi_(phi)
{
    
}


/*
 *
 */
void
RegularVertexGrid::addVertex(size_t i, size_t j, gmx::RVec vertex)
{
    std::pair<size_t, size_t> key(i, j);
    vertices_[key] = vertex;

/*
    std::cout<<"i = "<<i<<"  "
             <<"j = "<<j<<"  "
             <<"x = "<<vertices_[key][0]<<"  "
             <<"y = "<<vertices_[key][1]<<"  "
             <<"z = "<<vertices_[key][2]<<"  "
             <<"key.1 = "<<key.first<<"  "
             <<"key.2 = "<<key.second<<"  "
             <<std::endl;*/
}



/*
 *
 */
void
RegularVertexGrid::interpolateMissing()
{
    // loop over grid points:
    for(size_t i = 0; i < s_.size(); i++)
    {
        for(size_t j = 0; j < phi_.size(); j++)
        {
            // is value present?
            std::pair<size_t, size_t> key(i, j);
            auto it = vertices_.find(key);
            if( it == vertices_.end() )
            {
                // TODO find nearest neighbours on grid


                // TODO bilinear interpolation.
            }
        }
    }
}


/*
 *
 */
std::vector<gmx::RVec>
RegularVertexGrid::vertices()
{
    // build linearly indexed vector from dual indexed map:
    std::vector<gmx::RVec> vert;
    vert.reserve(vertices_.size());
    for(size_t i = 0; i < s_.size(); i++)
    {
        for(size_t j = 0; j < phi_.size(); j++)
        {
            std::pair<size_t, size_t> key(i, j);
            if( vertices_.find(key) != vertices_.end() )
            {
                vert.push_back(vertices_[key]); 
/*                std::cout<<"i = "<<i<<"  "
                         <<"j = "<<j<<"  "
                         <<"x = "<<vertices_[key][0]<<"  "
                         <<"y = "<<vertices_[key][1]<<"  "
                         <<"z = "<<vertices_[key][2]<<"  "
                         <<"key.1 = "<<key.first<<"  "
                         <<"key.2 = "<<key.second<<"  "
                         <<std::endl;*/
            }
            else
            {
                throw std::logic_error("Invalid vertex reference encountered.");
            }
        }
    }

    return vert;
}


/*
 *
 */
std::vector<gmx::RVec>
RegularVertexGrid::normals()
{
    std::vector<gmx::RVec> norm;
    return norm;
}


/*
 *
 */
std::vector<WavefrontObjFace>
RegularVertexGrid::faces()
{
    // sanity checks:
    if( phi_.size() * s_.size() != vertices_.size() )
    {
        std::cout<<"phi.size = "<<phi_.size()<<"  "
                 <<"s.size = "<<s_.size()<<"  "
                 <<"vert.size = "<<vertices_.size()<<"  "
                 <<std::endl;
        throw std::logic_error("Cannot generate faces on incomplete grid.");
    }

    // preallocate face vector:
    std::vector<WavefrontObjFace> faces;
    faces.reserve(phi_.size()*s_.size());

    // loop over grid:
    for(size_t i = 0; i < s_.size() - 1; i++)
    {
        for(size_t j = 0; j < phi_.size() - 1; j++)
        {
            // calculate linear indices:
            int kbl = i*phi_.size() + j; 
            int kbr = kbl + 1;
            int ktl = kbl + phi_.size();
            int ktr = kbr + phi_.size();

            // two faces per square:
            faces.push_back( WavefrontObjFace(
                    {kbl + 1, ktr + 1, ktl + 1}) );
            faces.push_back( WavefrontObjFace(
                    {kbl + 1, kbr + 1, ktr + 1}) );
        }
    }

    // wrap around:
    for(size_t i = 0; i < s_.size() - 1; i++)
    {
        // calculate linear indices:
        int kbl = i*phi_.size() + phi_.size() - 1; 
        int kbr = i*phi_.size();
        int ktl = kbl + phi_.size();
        int ktr = kbr + phi_.size();

        // two faces per square:
        faces.push_back( WavefrontObjFace(
                {kbl + 1, ktr + 1, ktl + 1}) );
        faces.push_back( WavefrontObjFace(
                {kbl + 1, kbr + 1, ktr + 1}) );
    }

    // return face vector:
    return faces;
}



/*
 * Constructor.
 */
MolecularPathObjExporter::MolecularPathObjExporter()
{
    
}


void
MolecularPathObjExporter::operator()(std::string fileName,
                                     MolecularPath &molPath)
{
    
//    real d = 0.1;
//    real r = 1;

    bool useNormals = true;
    real extrapDist = 0.0;

    int nLen = 50;
    int nPhi = 30;

    int numPhi = 30;
    int numLen = 50;

//    real deltaLen = molPath.length() / (nLen - 1);
    real deltaPhi = 2.0*PI_/(nPhi - 1);


    // generate a grid of vertices:
    RegularVertexGrid grid = generateGrid(molPath, numLen, numPhi, extrapDist);

    // correct mesh:
    grid.interpolateMissing();
 




/*

    // get tangents and points on centre line to centre line:
    std::vector<gmx::RVec> tangents = molPath.sampleNormTangents(nLen, extrapDist);
    std::vector<gmx::RVec> centrePoints = molPath.samplePoints(nLen, extrapDist);
    std::vector<real> radii = molPath.sampleRadii(nLen, extrapDist);

    // generate normals:


    // preallocate output vertices:
//    std::vector<gmx::RVec> vertices;
//    vertices.reserve(tangents.size()*nPhi);
//    std::vector<gmx::RVec> vertexNormals;
//    vertexNormals.reserve(vertices.size()*nPhi);


    // construct vertices around first point:
    gmx::RVec normal = orthogonalVector(tangents[0]);
    unitv(normal, normal);
    auto ring = vertexRing(
            centrePoints[0],
            tangents[0],
            normal,
            radii[0],
            deltaPhi,
            nPhi);

    // add vertices and normals to containers:
    
    vertices.insert(
            vertices.end(), 
            ring.first.begin(), 
            ring.first.end());
    vertexNormals.insert(
            vertexNormals.end(), 
            ring.second.begin(), 
            ring.second.end());
    

    SplineCurve1D pathRadius = molPath.pathRadius();    
    SplineCurve3D centreLine = molPath.centreLine();

    gmx::RVec prevTangent;

    std::vector<real> s;
    std::vector<real> phi;
    for(int i = 0; i < numLen; i++)
    {
        s.push_back(molPath.sLo() + molPath.length()/numLen*i);
    }
    for(int i = 0; i < numPhi; i++)
    {
        phi.push_back(2.0*M_PI/numPhi*i);
    }
    RegularVertexGrid grid(s, phi);

    int vertIdxA = numPhi + 1;
    int vertIdxB = numPhi + 2;
    int vertIdxC = numPhi + 3;

    real eval = molPath.sLo();
    real evalStep = molPath.length()/numLen;
    size_t i = 0;
    while( i < numLen )
    {
        eval = s[i];
        real crntRadius = pathRadius.evaluate(eval, 0);
        gmx::RVec crntCentre = centreLine.evaluate(eval, 0);
        gmx::RVec crntTangent = centreLine.tangentVec(eval);
        gmx::RVec crntNormal = orthogonalVector(crntTangent);

        // find axis of tangent rotation:
        gmx::RVec tangentRotAxis;
        cprod(prevTangent, crntTangent, tangentRotAxis);

        // find tangent angle of rotation:
        real tangentRotCos = iprod(crntTangent, prevTangent);
        real tangentRotAxisLen = norm(tangentRotAxis);
        real tangentRotAngle = std::atan2(tangentRotAxisLen, tangentRotCos);

        // update normal by rotating it like the tangent:
        crntNormal = rotateAboutAxis(normal, tangentRotAxis, tangentRotAngle);


        // TODO
        // 0. pick numLen as power of two
        // 1. precompute tangents and normals and centres
        // 2. starting from end points, half intervals in s
        // (3. at midpoint, evaluate clashes) <-- later
        // 4. add vertices in this ring to grid

        
        // angular loop:
        real angleIncrement = 2.0*M_PI/(numPhi);
        for(size_t j = 0; j < numPhi; j++)
        {
            // rotate normal vector:
            gmx::RVec rotNormal = rotateAboutAxis(
                    crntNormal, 
                    crntTangent,
                    j*angleIncrement);

            // generate vertex:
            gmx::RVec vertex = crntCentre;
            vertex[XX] += crntRadius*rotNormal[XX];
            vertex[YY] += crntRadius*rotNormal[YY];
            vertex[ZZ] += crntRadius*rotNormal[ZZ];

            // add to grid:
            std::cout<<"x = "<<vertex[XX]<<"  "
                     <<"y = "<<vertex[YY]<<"  "
                     <<"z = "<<vertex[ZZ]<<"  "
                     <<std::endl;
            grid.addVertex(i, j, vertex);


            // add vertices, and normals to container:
//            vertices.push_back(vertex);
//            vertexNormals.push_back(rotNormal);
            vertIdxA++;
        }


        auto ring = vertexRing(
            crntCentre,
            crntTangent,
            crntNormal,
            crntRadius,
            deltaPhi,
            nPhi);
           
        for(int i = 0; i < ring.first.size(); i++)
        {
            gmx::RVec diff;
            rvec_sub(ring.first[i], centrePoints[i-1], diff);
            real angle = iprod(tangents[i-1], diff) / ( norm(tangents[i-1]) * norm(diff) );

            if( angle > 0 || true )
            {
                vertices.push_back(ring.first[i]);
                vertexNormals.push_back(ring.second[i]);
            }
        }


        prevTangent = crntTangent;
//        eval += evalStep;
        i++;
    }
*/

    // obtain vertices, normals, and faces from grid:
    auto vertices = grid.vertices();
    auto faces = grid.faces();

    std::cout<<"vertices.size = "<<vertices.size()<<"  "
             <<"faces.size = "<<faces.size()<<"  "
             <<std::endl;

    

    // add faces to surface:
    WavefrontObjGroup surface("pore_surface");
    for(auto face : faces)
    {
/*        std::cout<<"face = "<<"  "
                 <<face.vertexIdx_[0]<<"  "
                 <<face.vertexIdx_[1]<<"  "
                 <<face.vertexIdx_[2]<<"  "
                 <<std::endl;*/
        surface.addFace(face);
    }
    
    // assemble OBJ object:
    WavefrontObjObject obj("pore");
    obj.addVertices(vertices);
//    obj.addVertexNormals(vertexNormals);
    obj.addGroup(surface);

    // scale object by factor of 10 to convert nm to Ang:
    obj.scale(10.0);
    obj.calculateCog();

    // create OBJ exporter and write to file:
    WavefrontObjExporter objExp;
    objExp.write(fileName, obj);

    
}


/*
 *
 */
std::vector<gmx::RVec>
MolecularPathObjExporter::generateNormals(
        const std::vector<gmx::RVec> &tangents)
{
    // preallocate container for normal vectors:
    std::vector<gmx::RVec> normals;
    normals.reserve(tangents.size());

    // generate initial normal:
    gmx::RVec normal = orthogonalVector(tangents[0]);
    unitv(normal, normal);
    normals.push_back(normal);
 
    // loop over tangent vectors and update normals:
    for(unsigned int i = 1; i < tangents.size(); i++)
    {
        // find axis of tangent rotation:
        gmx::RVec tangentRotAxis;
        cprod(tangents[i - 1], tangents[i], tangentRotAxis);

        // find tangent angle of rotation:
        real tangentRotCos = iprod(tangents[i], tangents[i - 1]);
        real tangentRotAxisLen = norm(tangentRotAxis);
        real tangentRotAngle = std::atan2(tangentRotAxisLen, tangentRotCos);

        // update normal by rotating it like the tangent:
        normal = rotateAboutAxis(normal, tangentRotAxis, tangentRotAngle);
        unitv(normal, normal);
        normals.push_back(normal);
    }

    // return normal vectors:
    return normals;
}


/*
 *
 */
RegularVertexGrid
MolecularPathObjExporter::generateGrid(
        MolecularPath &molPath,
        size_t numLen,
        size_t numPhi,
        real extrapDist)
{    
    // generate grid coordinates:
    auto s = molPath.sampleArcLength(numLen, extrapDist);
    std::vector<real> phi;
    phi.reserve(numPhi);
    for(size_t i = 0; i < numPhi; i++)
    {
        phi.push_back(i*2.0*M_PI/numPhi);
    }

    // generate grid from coordinates:
    RegularVertexGrid grid(s, phi);

    // sample centre points radii, and tangents along molecular path:
    auto centres = molPath.samplePoints(s);
    auto radii  = molPath.sampleRadii(s);
    auto tangents = molPath.sampleNormTangents(s);

    // sample normals along molecular path:
    auto normals = generateNormals(tangents);

    // generate vertices along molecular path:
    for(size_t i = 0; i < s.size(); i++)
    {        
        // angular loop:
        for(size_t j = 0; j < phi.size(); j++)
        {
            // rotate normal vector:
            gmx::RVec rotNormal = rotateAboutAxis(
                    normals[i], 
                    tangents[i],
                    phi[j]);

            // generate vertex:
            gmx::RVec vertex = centres[i];
            vertex[XX] += radii[i]*rotNormal[XX];
            vertex[YY] += radii[i]*rotNormal[YY];
            vertex[ZZ] += radii[i]*rotNormal[ZZ];

            // add to grid:
            grid.addVertex(i, j, vertex);
        }
    }
 
    // return grid:
    return grid;
}


/*
 *
 */
/*
void
MolecularPathObjExporter::operator()(std::string fileName,
                                     MolecularPath &molPath)
{
    
//    real d = 0.1;
//    real r = 1;

    bool useNormals = true;
    real extrapDist = 0.0;

    int nLen = 250;
    int nPhi = 30;


//    real deltaLen = molPath.length() / (nLen - 1);
    real deltaPhi = 2.0*PI_/(nPhi - 1);



    // get tangents and points on centre line to centre line:
    std::vector<gmx::RVec> tangents = molPath.sampleNormTangents(nLen, extrapDist);

    std::vector<gmx::RVec> centrePoints = molPath.samplePoints(nLen, extrapDist);
    std::vector<real> radii = molPath.sampleRadii(nLen, extrapDist);

    // preallocate output vertices:
    std::vector<gmx::RVec> vertices;
    vertices.reserve(tangents.size()*nPhi);
    std::vector<gmx::RVec> vertexNormals;
    vertexNormals.reserve(vertices.size()*nPhi);


    // construct vertices around first point:
    gmx::RVec normal = orthogonalVector(tangents[0]);
    unitv(normal, normal);
    auto ring = vertexRing(
            centrePoints[0],
            tangents[0],
            normal,
            radii[0],
            deltaPhi,
            nPhi);

    // add vertices and normals to containers:
    vertices.insert(
            vertices.end(), 
            ring.first.begin(), 
            ring.first.end());
    vertexNormals.insert(
            vertexNormals.end(), 
            ring.second.begin(), 
            ring.second.end());

    // follow spline curve and construct vertices around all other points:
    for(unsigned int i = 1; i < tangents.size(); i++)
    {
        // find axis of tangent rotation:
        gmx::RVec tangentRotAxis;
        cprod(tangents[i - 1], tangents[i], tangentRotAxis);

        // find tangent angle of rotation:
        real tangentRotCos = iprod(tangents[i], tangents[i - 1]);
        real tangentRotAxisLen = norm(tangentRotAxis);
        real tangentRotAngle = std::atan2(tangentRotAxisLen, tangentRotCos);

        // update normal by rotating it like the tangent:
        normal = rotateAboutAxis(normal, tangentRotAxis, tangentRotAngle);

        // construct sample points:
        ring = vertexRing(
                centrePoints[i],
                tangents[i],
                normal,
                radii[i],
                deltaPhi,
                nPhi);

        for(int i = 0; i < ring.first.size(); i++)
        {
            gmx::RVec diff;
            rvec_sub(ring.first[i], centrePoints[i-1], diff);
            real angle = iprod(tangents[i-1], diff) / ( norm(tangents[i-1]) * norm(diff) );

            if( angle > 0 )
            {
//                vertices.push_back(ring.first[i]);
//                vertexNormals.push_back(ring.second[i]);
            }
        }

        
        // add vertices and normals to containers:
        vertices.insert(
                vertices.end(), 
                ring.first.begin(), 
                ring.first.end());
        vertexNormals.insert(
                vertexNormals.end(), 
                ring.second.begin(), 
                ring.second.end());
    }


    std::cout<<"vertices.size = "<<vertices.size()<<"  "
             <<"nLen = "<<nLen<<"  "
             <<"nPhi = "<<nPhi<<"  "
             <<std::endl;

    // construct triangular faces:
    WavefrontObjGroup surface("pore_surface");
    for(int i = 0; i < vertices.size() / nPhi - 1; i++)
    {
        for(int j = 0; j < nPhi - 1; j++)
        {
            // calculate linear indeces:
            int kbl = i*nPhi + j; 
            int kbr = kbl + 1;
            int ktl = kbl + nPhi;
            int ktr = kbr + nPhi;

            // two triangles for each square:
            std::unique_ptr<WavefrontObjFace> faceA;
            std::unique_ptr<WavefrontObjFace> faceB;
            if( useNormals )
            {
                faceA.reset(new WavefrontObjFace({kbl + 1, ktr + 1, ktl + 1},
                                                 {kbl + 1, ktl + 1, ktr + 1}));
                faceB.reset(new WavefrontObjFace({kbl + 1, kbr + 1, ktr + 1},
                                                 {kbl + 1, kbr + 1, ktr + 1}));
            }
            else
            {
                faceA.reset(new WavefrontObjFace({kbl + 1, ktr + 1, ktl + 1}));
                faceB.reset(new WavefrontObjFace({kbl + 1, kbr + 1, ktr + 1}));
            }

            // add faces to overall surface:
            surface.addFace(*faceA);
            surface.addFace(*faceB);
        }
    }


    // assemble OBJ object:
    WavefrontObjObject obj("pore");
    obj.addVertices(vertices);
    obj.addVertexNormals(vertexNormals);
    obj.addGroup(surface);

    // scale object by factor of 10 to convert nm to Ang:
    obj.scale(10.0);
    obj.calculateCog();

    // create OBJ exporter and write to file:
    WavefrontObjExporter objExp;
    objExp.write(fileName, obj);

    
}
*/


/*
 *
 */
int
MolecularPathObjExporter::numPlanarVertices(real &d, real &r)
{
    return std::max(static_cast<int>(std::ceil(PI_/(2.0*std::acos(1.0 - d*d/(2.0*r*r))))), 4);
}


/*
 *
 */
std::pair<std::vector<gmx::RVec>, std::vector<gmx::RVec>>
MolecularPathObjExporter::vertexRing(gmx::RVec base,
                                     gmx::RVec tangent,
                                     gmx::RVec normal,
                                     real radius,
                                     real angleIncrement,
                                     size_t nIncrements)
{
    // make sure input normal vector is normalised:
    unitv(normal, normal);

    // preallocate vertex and normal vectors:
    std::vector<gmx::RVec> vertices;
    vertices.reserve(nIncrements);
    std::vector<gmx::RVec> normals;
    vertices.reserve(nIncrements);

    // sample vertices in a ring around the base point: 
    for(size_t j = 0; j < nIncrements; j++)
    {
        // rotate normal vector:
        gmx::RVec rotNormal = rotateAboutAxis(normal, tangent, j*angleIncrement);
        normals.push_back(rotNormal);

        // generate vertex:
        gmx::RVec vertex = base;
        vertex[XX] += radius*rotNormal[XX];
        vertex[YY] += radius*rotNormal[YY];
        vertex[ZZ] += radius*rotNormal[ZZ];
        vertices.push_back(vertex);
    }

    // return ring of vertices:
    std::pair<std::vector<gmx::RVec>, std::vector<gmx::RVec>> ret;
    ret.first = vertices;
    ret.second = normals;
    return ret;
}


/*
 *
 */
gmx::RVec
MolecularPathObjExporter::orthogonalVector(gmx::RVec vec)
{
    // find first nonzero element in vector:
    int idxNonZero = -1;
    for(int i = 0; i < 3; i++)
    {
        if( std::abs(vec[i]) > std::numeric_limits<real>::epsilon() )
        {
            idxNonZero = i;
            break;
        }
    }

    // sanity check:
    if( idxNonZero == -1 )
    {
        std::cerr<<"ERROR: Can not find orthogonal to null vector!"<<std::endl;
        std::cerr<<"vec = "<<vec[0]<<" "<<vec[1]<<" "<<vec[2]<<std::endl;
        std::abort();
    }

    // find index for switching:
    int idxSwitch = (idxNonZero + 1) % 3;

    // construct non-colinear vector by element switching:
    gmx::RVec otherVec = vec;
    otherVec[idxNonZero] = vec[idxSwitch];
    otherVec[idxSwitch] = -vec[idxNonZero];

    // construct orthogonal vector via cross product:
    gmx::RVec orthVec;
    cprod(vec, otherVec, orthVec);

    // return a vector orthogonal to input vector:
    return orthVec;
}


/*
 *
 */
gmx::RVec
MolecularPathObjExporter::rotateAboutAxis(gmx::RVec vec, 
                                          gmx::RVec axis,
                                          real angle)
{
    // evaluate trigonometrix functions of rotation angle:
    const real COS = std::cos(angle);
    const real SIN = std::sin(angle);

    // construct rotation matrix:
    matrix rotMat;
    rotMat[XX][XX] = COS + axis[XX]*axis[XX]*(1.0 - COS);
    rotMat[XX][YY] = axis[XX]*axis[YY]*(1.0 - COS) - axis[ZZ]*SIN;
    rotMat[XX][ZZ] = axis[XX]*axis[ZZ]*(1.0 - COS) + axis[YY]*SIN;
    rotMat[YY][XX] = axis[YY]*axis[XX]*(1.0 - COS) + axis[ZZ]*SIN;
    rotMat[YY][YY] = COS + axis[YY]*axis[YY]*(1 - COS);
    rotMat[YY][ZZ] = axis[YY]*axis[ZZ]*(1.0 - COS) - axis[XX]*SIN;
    rotMat[ZZ][XX] = axis[ZZ]*axis[XX]*(1.0 - COS) - axis[YY]*SIN;
    rotMat[ZZ][YY] = axis[ZZ]*axis[YY]*(1.0 - COS) + axis[XX]*SIN;
    rotMat[ZZ][ZZ] = COS + axis[ZZ]*axis[ZZ]*(1.0 - COS);

    // rotate vector:
    gmx::RVec rotVec;
    mvmul(rotMat, vec, rotVec);

    // return rotated vector:
    return rotVec;
}

