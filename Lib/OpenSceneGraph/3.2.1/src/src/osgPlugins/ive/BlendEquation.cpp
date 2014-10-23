/**********************************************************************
 *
 *    FILE:            BlendEquation.cpp
 *
 *    DESCRIPTION:    Read/Write osg::BlendEquation in binary format to disk.
 *
 *    CREATED BY:        Auto generated by iveGenerated
 *                    and later modified by Rune Schmidt Jensen.
 *
 *    HISTORY:        Created 21.3.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "BlendEquation.h"
#include "Object.h"

using namespace ive;

void BlendEquation::write(DataOutputStream* out){

    // Write BlendEquation's identification.
    out->writeInt(IVEBLENDEQUATION);

    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object*  obj = dynamic_cast<osg::Object*>(this);
    if(obj){
        ((ive::Object*)(obj))->write(out);
    }
    else
        out_THROW_EXCEPTION("BlendEquation::write(): Could not cast this osg::BlendEquation to an osg::Object.");
    // Write BlendEquation's properties.

    // Write source
    if ( out->getVersion() >= VERSION_0040 )
    {
        out->writeInt(getEquationRGB());
        out->writeInt(getEquationAlpha());
    }
    else
    {
        out->writeInt(getEquation());
    }
}

void BlendEquation::read(DataInputStream* in){
    // Peek on BlendEquation's identification.
    int id = in->peekInt();
    if(id == IVEBLENDEQUATION){
        // Read BlendEquation's identification.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if(obj){
            ((ive::Object*)(obj))->read(in);
        }
        else
            in_THROW_EXCEPTION("BlendEquation::read(): Could not cast this osg::BlendEquation to an osg::Object.");
        // Read BlendEquation's properties

        // Read source
        if ( in->getVersion() >= VERSION_0040 )
        {
            setEquationRGB(osg::BlendEquation::Equation(in->readInt()));
            setEquationAlpha(osg::BlendEquation::Equation(in->readInt()));
        }
        else
        {
            setEquation(osg::BlendEquation::Equation(in->readInt()));
        }
    }
    else{
        in_THROW_EXCEPTION("BlendEquation::read(): Expected BlendEquation identification.");
    }
}
