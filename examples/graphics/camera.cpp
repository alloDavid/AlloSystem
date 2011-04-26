/*
Allocore Example: Camera

Description:
A very simple app that shows how the camera can be used (with Drawable and Nav 
classes).

Author:
Ritesh Lala, 4/20/2011
Modified by Lance Putnam, 4/25/2011
*/

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"

using namespace al;

struct MyWindow : public Window, public Drawable, public Nav{
    
    MyWindow()
	:	nav(Vec3d(0,0,-2), 0.8)
	{
		cam.fovy(90);	// set field of view angle
		
		add(new StandardWindowKeyControls);
		add(new NavInputControl(nav));

		// create some shapes to draw
		shapes.primitive(gl.TRIANGLES);
		
		for(int j=0; j<1000; ++j){
			int Nv = addCube(shapes);

			Mat4f xfm;
			xfm.setIdentity();
			scale(xfm, rnd::uniform(2.,0.2));
			translate(xfm, Vec3f(rnd::uniformS(20.), rnd::uniformS(20.), rnd::uniformS(20.)));
			shapes.transform(xfm, shapes.vertices().size()-Nv);

			for(int i=0; i<Nv; ++i){
				float v = float(i)/Nv;
				shapes.color(HSV(0.2*v, 1-v*0.5, 1));
			}
		}
	}
    
	bool onFrame(){

        nav.step();

		gl.depthTesting(true);

		stereo.draw(gl, cam, nav, Viewport(width(), height()), *this);

		return true;
	}
    
	virtual void onDraw(Graphics& gl){		
		gl.draw(shapes);
	}
	
	virtual bool onKeyDown(const Keyboard& k){
		switch(k.key()){
			case 'f': cam.fovy(cam.fovy()-5); break;
			case 'g': cam.fovy(cam.fovy()+5); break;
			default:;
		}
		return true;
	}
    
    GraphicsGL gl;
	Camera cam;
	Nav nav;
	Stereographic stereo;
	Mesh shapes;
};

int main(){
	MyWindow win1;
    
    win1.create(Window::Dim(800, 600), "Allocore Example: Camera");

	MainLoop::start();
	return 0;
}