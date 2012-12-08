#ifndef AL_OMNIAPP_H
#define AL_OMNIAPP_H

#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "alloutil/al_FPS.hpp"
#include "alloutil/al_OmniStereo.hpp"

#define DEVICE_SERVER_RECEIVE_PORT 12000
#define DEVICE_SERVER_SEND_PORT (DEVICE_SERVER_RECEIVE_PORT+1)
#define DEVICE_SERVER_IP_ADDRESS "BOSSANOVA"

namespace al {

class OmniApp : public Window, public osc::PacketHandler, public FPS, public OmniStereo::Drawable {
public:
	OmniApp(std::string name = "omniapp");
	virtual ~OmniApp();

	void start();
	
	virtual void onDraw(Graphics& gl) {}
	virtual void onAnimate(al_sec dt) {}
	virtual void onSound(AudioIOData& io) {}
	virtual void onMessage(osc::Message& m);
	
	const AudioIO&		audioIO() const { return mAudioIO; }
	AudioIO&			audioIO(){ return mAudioIO; }
	
	const Lens&			lens() const { return mLens; }
	Lens&				lens() { return mLens; }

	const Graphics&		graphics() const { return mGraphics; }
	Graphics&			graphics(){ return mGraphics; }
	
	const Nav&			nav() const { return mNav; }
	Nav&				nav(){ return mNav; }
	
	ShaderProgram&		shader() { return mShader; }

	const std::string&	name() const { return mName; }
	OmniApp&			name(const std::string& v){ mName=v; return *this; }

	osc::Recv&			oscRecv(){ return mOSCRecv; }
	osc::Send&			oscSend(){ return mOSCSend; }
	
	void initWindow(
		const Window::Dim& dims = Window::Dim(800, 400),
		const std::string title = "OmniApp",
		double fps = 60,
		Window::DisplayMode mode = Window::DEFAULT_BUF);
			
	void initAudio(
		double audioRate=44100, int audioBlockSize=256
	);
	
	void initAudio(
		double audioRate, int audioBlockSize,
		int audioOutputs, int audioInputs
	);
	void initOmni(std::string path = "/home/sphere/code/allosphere/calibration-current/");
	
	void sendHandshake();
	void sendDisconnect();
	
	virtual bool onCreate();
	virtual bool onFrame();
	virtual void onDrawOmni(OmniStereo& omni);
	
	virtual std::string	vertexCode();
	virtual std::string	fragmentCode();

protected:

	AudioIO mAudioIO;
	OmniStereo mOmni;
	
	Lens mLens;
	Graphics mGraphics;
	
	ShaderProgram mShader;
	
	// control
	Nav mNav;
	NavInputControl mNavControl;
	StandardWindowKeyControls mStdControls;
	osc::Recv mOSCRecv;
	osc::Send mOSCSend;
	
	std::string mName;
	std::string mHostName;
	
	static void AppAudioCB(AudioIOData& io);
};


// INLINE IMPLEMENTATION //

inline OmniApp::OmniApp(std::string name)
:	mNavControl(mNav),
	mOSCRecv(DEVICE_SERVER_SEND_PORT),
	mOSCSend(DEVICE_SERVER_RECEIVE_PORT, DEVICE_SERVER_IP_ADDRESS) {
	mHostName = Socket::hostName();
	mName = name;
	
	Window::append(mStdControls);
	Window::append(mNavControl);
	
	oscRecv().bufferSize(32000);
	oscRecv().handler(*this);
	sendHandshake();
										
	initAudio();
	initWindow();
	initOmni();
	
	lens().near(0.01).far(40).eyeSep(0.03);
	nav().smooth(0.8);
}

inline OmniApp::~OmniApp() {
	sendDisconnect();
}

inline void OmniApp::initOmni(std::string path) {
	mOmni.configure(path, mHostName);
	if (mOmni.activeStereo()) {
		mOmni.mode(OmniStereo::ACTIVE).stereo(true);
	}
}

inline void OmniApp::initWindow(
	const Window::Dim& dims, const std::string title, double fps, Window::DisplayMode mode) {
	Window::dimensions(dims);
	Window::title(title);
	Window::fps(fps);
	Window::displayMode(mode);
}

inline void OmniApp::initAudio(
	double audioRate, int audioBlockSize
) {
	mAudioIO.callback = AppAudioCB;
	mAudioIO.user(this);
	mAudioIO.framesPerSecond(audioRate);
	mAudioIO.framesPerBuffer(audioBlockSize);
}

inline void OmniApp::initAudio(
	double audioRate, int audioBlockSize,
	int audioOutputs, int audioInputs
) {
	initAudio(audioRate, audioBlockSize);
	mAudioIO.channelsOut(audioOutputs);
	mAudioIO.channelsIn(audioInputs);
}

inline void OmniApp::sendHandshake(){
	oscSend().send("/handshake", name(), oscRecv().port());
}

inline void OmniApp::sendDisconnect(){
	oscSend().send("/disconnectApplication", name());
}

inline void OmniApp::start() {
	if (mOmni.activeStereo()) {
		Window::displayMode(Window::displayMode() | Window::STEREO_BUF);
	}
	
	if(oscSend().opened()) sendHandshake();
	
	create();
	
	if (mOmni.fullScreen()) {
		fullScreen(true);
		cursorHide(true);
	}
	
	mAudioIO.start();
	Main::get().start();
}

inline bool OmniApp::onCreate() {
	mOmni.onCreate();
	
	Shader vert, frag;
	vert.source(OmniStereo::glsl() + vertexCode(), Shader::VERTEX).compile();
	vert.printLog();
	frag.source(fragmentCode(), Shader::FRAGMENT).compile();
	frag.printLog();
	mShader.attach(vert).attach(frag).link();
	mShader.printLog();
  mShader.begin();
  mShader.uniform("lighting", 1.0);
  mShader.end();
	
	return true;
}

inline bool OmniApp::onFrame() {
	FPS::onFrame();

	while(oscRecv().recv()) {}
	
	nav().step();
	
	onAnimate(dt);
	
	Viewport vp(width(), height());
	mOmni.onFrame(*this, lens(), nav(), vp);
	
	return true;
}

inline void OmniApp::onDrawOmni(OmniStereo& omni) {
	graphics().error("start onDraw");
	
	mShader.begin();
	mOmni.uniforms(mShader);
	
	onDraw(graphics());
	
	mShader.end();
}

inline void OmniApp::onMessage(osc::Message& m) {
	float x;
	if (m.addressPattern() == "/mx") {
		m >> x;
		nav().moveR(-x);

	} else if (m.addressPattern() == "/my") {
		m >> x;
		nav().moveU(x);

	} else if (m.addressPattern() == "/mz") {
		m >> x;
		nav().moveF(x);

	} else if (m.addressPattern() == "/tx") {
		m >> x;
		nav().spinR(x * -0.02);

	} else if (m.addressPattern() == "/ty") {
		m >> x;
		nav().spinU(x * 0.02);

	} else if (m.addressPattern() == "/tz") {
		m >> x;
		nav().spinF(x * -0.02);

	}
}

inline std::string	OmniApp::vertexCode() {
	return AL_STRINGIFY(
		varying vec4 color;
		varying vec3 normal, lightDir, eyeVec;
		void main(){
			color = gl_Color;
			vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
			normal = gl_NormalMatrix * gl_Normal;
			vec3 V = vertex.xyz;
			eyeVec = normalize(-V);
			lightDir = normalize(vec3(gl_LightSource[0].position.xyz - V));
			gl_Position = omni_render(vertex); 
		}
	);
}

inline std::string OmniApp::fragmentCode() {
	return AL_STRINGIFY(
    uniform float lighting;
		varying vec4 color;
		varying vec3 normal, lightDir, eyeVec;
    	void main() {
			vec4 final_color = color * gl_LightSource[0].ambient;
			vec3 N = normalize(normal);
			vec3 L = lightDir;
			float lambertTerm = max(dot(N, L), 0.0);
      final_color += gl_LightSource[0].diffuse * color * lambertTerm;
      vec3 E = eyeVec;
      vec3 R = reflect(-L, N);
      float spec = pow(max(dot(R, E), 0.0), 0.9 + 1e-20);
      final_color += gl_LightSource[0].specular * spec;
			gl_FragColor = mix(color, final_color, lighting);
		}
	);
}

inline void OmniApp::AppAudioCB(AudioIOData& io){
	OmniApp& app = io.user<OmniApp>();
	io.frame(0);
	app.onSound(io);
}

}

#endif