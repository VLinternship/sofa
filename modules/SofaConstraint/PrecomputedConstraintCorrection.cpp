/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2018 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#define SOFA_COMPONENT_CONSTRAINTSET_PRECOMPUTEDCONSTRAINTCORRECTION_CPP

#include "PrecomputedConstraintCorrection.inl"

#include <sofa/core/ObjectFactory.h>

namespace sofa
{

namespace component
{

namespace constraintset
{


template<>
SOFA_CONSTRAINT_API void PrecomputedConstraintCorrection< defaulttype::Rigid3Types >::rotateConstraints(bool back)
{
    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::position())->getValue();
    const VecCoord& x0 = this->mstate->read(core::ConstVecCoordId::restPosition())->getValue();
    helper::WriteAccessor<Data<MatrixDeriv> > cData = *this->mstate->write(core::MatrixDerivId::constraintJacobian());
    MatrixDeriv& c = cData.wref();

    // On fait tourner les normales (en les ramenant dans le "pseudo" repere initial)

    MatrixDerivRowIterator rowItEnd = c.end();

    for (MatrixDerivRowIterator rowIt = c.begin(); rowIt != rowItEnd; ++rowIt)
    {
        MatrixDerivColIterator colItEnd = rowIt.end();

        for (MatrixDerivColIterator colIt = rowIt.begin(); colIt != colItEnd; ++colIt)
        {
            Deriv& n = colIt.val();
            const unsigned int localRowNodeIdx = colIt.index();

            sofa::defaulttype::Quat q;
            if (m_restRotations.getValue())
                q = x[localRowNodeIdx].getOrientation() * x0[localRowNodeIdx].getOrientation().inverse();
            else
                q = x[localRowNodeIdx].getOrientation();

            sofa::defaulttype::Vec3d n_i = q.inverseRotate(getVCenter(n));
            sofa::defaulttype::Vec3d wn_i= q.inverseRotate(getVOrientation(n));

            if(back)
            {
                n_i = q.rotate(getVCenter(n));
                wn_i= q.rotate(getVOrientation(n));
            }

            // on passe les normales du repere global au repere local
            getVCenter(n) = n_i;
            getVOrientation(n) = wn_i;
        }
    }
}



template<>
SOFA_CONSTRAINT_API void PrecomputedConstraintCorrection<defaulttype::Vec1Types>::rotateConstraints(bool /*back*/)
{
}



template<>
SOFA_CONSTRAINT_API void PrecomputedConstraintCorrection<defaulttype::Rigid3Types>::rotateResponse()
{
    helper::WriteAccessor<Data<VecDeriv> > dxData = *this->mstate->write(core::VecDerivId::dx());
    VecDeriv& dx = dxData.wref();
    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::position())->getValue();
    const VecCoord& x0 = this->mstate->read(core::ConstVecCoordId::restPosition())->getValue();
    for(unsigned int j = 0; j < dx.size(); j++)
    {
        // on passe les deplacements du repere local (au repos) au repere global
        Deriv temp ;
        sofa::defaulttype::Quat q;
        if (m_restRotations.getValue())
            q = x[j].getOrientation() * x0[j].getOrientation().inverse();
        else
            q = x[j].getOrientation();

        getVCenter(temp)		= q.rotate(getVCenter(dx[j]));
        getVOrientation(temp)  = q.rotate(getVOrientation(dx[j]));
        dx[j] = temp;
    }
}


template<>
SOFA_CONSTRAINT_API void PrecomputedConstraintCorrection<defaulttype::Vec1Types>::rotateResponse()
{
}


template<>
SOFA_CONSTRAINT_API void PrecomputedConstraintCorrection<defaulttype::Vec1Types>::draw(const core::visual::VisualParams* )
{
}

template<>
SOFA_CONSTRAINT_API void PrecomputedConstraintCorrection<defaulttype::Rigid3Types>::draw(const core::visual::VisualParams* )
{
}




using namespace sofa::defaulttype;

int PrecomputedConstraintCorrectionClass = core::RegisterObject("Component computing constraint forces within a simulated body using the compliance method.")
        .add< PrecomputedConstraintCorrection<Vec3Types> >()
        .add< PrecomputedConstraintCorrection<Vec1Types> >()
        .add< PrecomputedConstraintCorrection<Rigid3Types> >()

        ;

template class SOFA_CONSTRAINT_API PrecomputedConstraintCorrection<Vec3Types>;
template class SOFA_CONSTRAINT_API PrecomputedConstraintCorrection<Vec1Types>;
template class SOFA_CONSTRAINT_API PrecomputedConstraintCorrection<Rigid3Types>;


} // namespace collision

} // namespace component

} // namespace sofa
