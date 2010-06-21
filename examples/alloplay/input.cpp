

#include <math.h>
#include <stdio.h>
#include <stdarg.h>

//#include "Gamma/Gamma.h"
//#include "Gamma/arr.h"
//#include "Gamma/gen.h"
//#include "Gamma/fil.h"
//#include "Gamma/ipl.h"
//#include "Gamma/mem.h"
//#include "Gamma/scl.h"
//#include "Gamma/tbl.h"
//#include "Gamma/rnd.h"
//#include "Gamma/Constants.h"
//#include "Gamma/Conversion.h"
//#include "Gamma/Dims.h"
//#include "Gamma/Containers.h"
//#include "Gamma/Strategy.h"
//#include "Gamma/Ambisonics.h"
//#include "Gamma/AudioIO.h"
//#include "Gamma/Chaos.h"
//#include "Gamma/Delay.h"
//#include "Gamma/DFT.h"
//#include "Gamma/FFT.h"
//#include "Gamma/Envelope.h"
//#include "Gamma/Noise.h"
//#include "Gamma/Oscillator.h"
//#include "Gamma/Sampler.h"
//#include "Gamma/SoundFile.h"
//#include "Gamma/Sync.h"
//#include "Gamma/Effects.h"
//#include "Gamma/File.h"
//#include "Gamma/Particle.h"
////#include "Gamma/Serialize.h"
//#include "Gamma/SmartObject.h"
//#include "Gamma/Thread.h"
//#include "Gamma/Timer.h"
//#include "Gamma/Types.h"
//#include "Gamma/UnitMapper.h"

#define AL_OSX
#include "system/al_Config.h"
#include "system/al_MainLoop.h"
#include "system/al_Printing.hpp"
#include "system/al_Thread.h"
#include "system/al_Thread.hpp"
#include "system/al_Time.h"
#include "system/al_Time.hpp"
#include "types/al_Buffer.hpp"
#include "types/al_Color.hpp"
#include "types/al_Conversion.hpp"
#include "types/al_MsgTube.hpp"
#include "types/al_types.hpp"
//#include "protocol/al_Graphics.hpp"
#include "protocol/al_OSC.hpp"
#include "protocol/al_OSCAPR.hpp"
#include "protocol/al_Serialize.h"
#include "protocol/al_Serialize.hpp"
#include "math/al_Complex.hpp"
#include "math/al_Constants.hpp"
#include "math/al_Functions.hpp"
#include "math/al_Generators.hpp"
#include "math/al_Interpolation.hpp"
#include "math/al_Plane.hpp"
#include "math/al_Random.hpp"
#include "math/al_Quat.h"
#include "math/al_Quat.hpp"
#include "math/al_Vec.hpp"
#include "io/al_AudioIO.hpp"
#include "io/al_File.hpp"
#include "io/al_Socket.hpp"
#include "io/al_WindowGL.hpp"
#include "spatial/al_Camera.hpp"
#include "graphics/al_Config.h"
#include "graphics/al_Common.hpp"
#include "graphics/al_Debug.hpp"
#include "graphics/al_GPUObject.hpp"
#include "graphics/al_Light.hpp"
#include "graphics/al_Shader.hpp"
#include "graphics/al_Texture.hpp"

#ifdef __cplusplus

class Foo {
	Foo() {
		printf("MADE A FOO!");
	}
};
Foo foo;

extern "C" {
#endif


extern int test(int x) {
	printf("test %d\n", (int)fabs(3.14));
	printf("test-1 %d\n", (int)fabs(3.14));
	printf("test-2 %d\n", (int)fabs(3.14));
	printf("test-3 %d\n", (int)fabs(3.14));
	printf("test-4 %d\n", (int)fabs(3.14));
	return 0;
}

extern int test2(int x) {
	printf("test2 %d\n", (int)fabs(3.14));
	return 0;
}

void callback(al::AudioIOData &io) {
	printf(".");
}

al::AudioIO io; //(64, 44100.0, callback);
	
int main(int ac, char ** av) {

	printf("open %d\n", io.open());
	printf("start %d\n", io.start());
	al_sleep(1);
	io.print();								///< Prints info about current i/o devices to stdout.
	io.printError();
	
//	//gam::Accum<> acc;
//	
//	float buf[64];
//	gam::NoisePink<> z;
//	for (int i=0; i<64; i++) {
//		buf[i] = sin((float)i/64.*6.28); //z();
//	}
//	
//	gam::RFFT<float> c(64);
//	c.forward(buf);
//	
//	for (int i=0; i<64; i++) {
//		printf("%i %f\n", i, buf[i]);
//	}
	
	
	test(0);
	test2(1);
	
	al_sleep(3);
	printf("cpu %f\n", io.cpu());
	io.close();
	
	return 0;
}	

#ifdef __cplusplus
}
#endif
