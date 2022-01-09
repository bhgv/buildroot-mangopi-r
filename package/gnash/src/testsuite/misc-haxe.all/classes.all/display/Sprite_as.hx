// Sprite_as.hx:  ActionScript 3 "Sprite" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090514 by "rob". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.display.Sprite;
import flash.display.MovieClip;
import flash.display.DisplayObject;
import flash.display.Graphics;
import flash.media.SoundTransform;
import flash.geom.Rectangle;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Sprite_as {
    static function main() {
        
#if flash9
        var x1:Sprite = new Sprite();
        var x2:Sprite = new Sprite();

        // Make sure we actually get a valid class        
        if (Std.is(x1, Sprite)) {
            DejaGnu.pass("Sprite class exists");
        } else {
            DejaGnu.fail("Sprite lass doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.buttonMode) == ValueType.TBool) {
	    DejaGnu.pass("Sprite::buttonMode property exists");
	} else {
	    DejaGnu.fail("Sprite::buttonMode property doesn't exist");
	}
//FIXME:dropTarget will be tested more extensively later.
//	var r1:Rectangle = new Rectangle(0,0,10,10);
//	x1.dropTarget = r1;
// 	if (Std.is(x1.dropTarget, DisplayObject)) {
// 	    DejaGnu.pass("Sprite::dropTarget property exists");
// 	} else {
// 	    DejaGnu.fail("Sprite::dropTarget property doesn't exist");
// 	}
 	if (Std.is(x1.graphics, Graphics)) {
 	    DejaGnu.pass("Sprite::graphics property exists");
 	} else {
 	    DejaGnu.fail("Sprite::graphics property doesn't exist");
 	}
 	x1.hitArea = x2;
 	if (Std.is(x1.hitArea, Sprite)) {
 	    DejaGnu.pass("Sprite::hitArea property exists");
 	} else {
 	    DejaGnu.fail("Sprite::hitArea property doesn't exist");
 	}
 	if (Std.is(x1.soundTransform, SoundTransform)) {
 	    DejaGnu.pass("Sprite::soundTransform property exists");
 	} else {
 	    DejaGnu.fail("Sprite::soundTransform property doesn't exist");
 	}
	if (Type.typeof(x1.useHandCursor) == ValueType.TBool) {
	    DejaGnu.pass("Sprite::useHandCursor property exists");
	} else {
	    DejaGnu.fail("Sprite::useHandCursor property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.startDrag) == ValueType.TFunction) {
	    DejaGnu.pass("Sprite::startDrag() method exists");
	} else {
	    DejaGnu.fail("Sprite::startDrag() method doesn't exist");
	}
	if (Type.typeof(x1.stopDrag) == ValueType.TFunction) {
	    DejaGnu.pass("Sprite::stopDrag() method exists");
	} else {
	    DejaGnu.fail("Sprite::stopDrag() method doesn't exist");
	}

#else
    DejaGnu.note("Sprite did not exist in versions prior to SWF9");
#end
        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();

    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

