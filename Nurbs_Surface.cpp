#include <rtaudio/RtAudio.h>
#include <GL/glut.h>
#include <vector>
#include <iostream>
#include <cmath>

// Constants
const int GRID_SIZE = 75; // Size of the grid (75x75)
const float GRID_SPAN = 10.0f; // Grid spans from -5.0 to 5.0
const int SAMPLE_RATE = 44100; // Audio sample rate
unsigned int BUFFER_FRAMES = 512; // Audio buffer size
const int NUM_SAMPLES = GRID_SIZE; // Number of points on the first line

// Variables for interaction
float angleX = 25.0, angleY = 0.0; // Rotation angles
float zoom = 1.0; // Zoom factor
int lastX = 0, lastY = 0; // Last mouse position
bool mouseLeftDown = false; // Left mouse button state
bool mouseRightDown = false; // Right mouse button state

// Control points for the surface
std::vector<std::vector<std::vector<GLfloat>>> ctrlpoints(GRID_SIZE, std::vector<std::vector<GLfloat>>(GRID_SIZE, std::vector<GLfloat>(3)));

// RtAudio variables
RtAudio audio;
std::vector<float> amplitudeBuffer(NUM_SAMPLES, 0.0f); // Circular buffer for amplitude values
int bufferIndex = 0; // Index for the circular buffer

// Function to generate control points dynamically
void generateControlPoints() {
    float x_step = GRID_SPAN / (GRID_SIZE - 1); // Step size for x-axis
    float z_step = GRID_SPAN / (GRID_SIZE - 1); // Step size for z-axis

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            ctrlpoints[i][j][0] = -GRID_SPAN / 2 + j * x_step; // x-coordinate
            ctrlpoints[i][j][1] = 0.0f; // y-coordinate (elevation)
            ctrlpoints[i][j][2] = -GRID_SPAN / 2 + i * z_step; // z-coordinate
        }
    }
}

// Audio callback function
int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                  double streamTime, RtAudioStreamStatus status, void *userData) {
    float *in = (float *)inputBuffer;

    // Calculate amplitude
    float localAmplitude = 0.0f;
    for (unsigned int i = 0; i < nBufferFrames; i++) {
        localAmplitude += fabs(in[i]);
    }
    localAmplitude /= nBufferFrames;

    // Store amplitude in the circular buffer
    amplitudeBuffer[bufferIndex] = localAmplitude;
    bufferIndex = (bufferIndex + 1) % NUM_SAMPLES; // Move to the next position in the buffer

    return 0;
}

// Display function
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();

    // Apply zoom
    glScalef(zoom, zoom, zoom);

    // Apply rotation
    glRotatef(angleX, 1.0, 0.0, 0.0);
    glRotatef(angleY, 0.0, 1.0, 0.0);

    // Update Y-values of the first line based on amplitude buffer
    for (int j = 0; j < GRID_SIZE; ++j) {
        int sampleIndex = (bufferIndex + j) % NUM_SAMPLES;
        ctrlpoints[0][j][1] = (amplitudeBuffer[sampleIndex] + 0.1f) * 15.0f; // Scale amplitude for visualization
    }

    // Propagate amplitude data to adjacent lines
    for (int i = GRID_SIZE - 1; i > 0; --i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            ctrlpoints[i][j][1] = ctrlpoints[i - 1][j][1];
        }
    }

    // Draw the grid (only horizontal lines)
    glBegin(GL_LINES);
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE - 1; ++j) {
            // Calculate green color based on amplitude
            float amplitude = ctrlpoints[i][j][1] / 20.0f; // Normalize amplitude to [0, 1]
            amplitude = std::min(1.0f, std::max(0.0f, amplitude)); // Clamp to [0, 1]
            float brightness = 0.8f + 0.2f * amplitude; // Ensure minimum brightness of 0.8
            glColor3f(0.0f, brightness, 0.0f); // Set color (green, with brightness based on amplitude)

            // Draw horizontal lines
            glVertex3f(ctrlpoints[i][j][0], ctrlpoints[i][j][1], ctrlpoints[i][j][2]);
            glVertex3f(ctrlpoints[i][j + 1][0], ctrlpoints[i][j + 1][1], ctrlpoints[i][j + 1][2]);
        }
    }
    glEnd();

    glPopMatrix();
    glutSwapBuffers(); // Use double buffering for smooth rendering
}

// Idle function for continuous updates
void idle() {
    glutPostRedisplay(); // Force the display function to be called
}

// Initialization function
void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);

    // Generate control points
    generateControlPoints();

    // Initialize RtAudio
    if (audio.getDeviceCount() < 1) {
        std::cerr << "No audio devices found!" << std::endl;
        exit(1);
    }

    // Set up audio stream parameters
    RtAudio::StreamParameters parameters;
    parameters.deviceId = audio.getDefaultInputDevice();
    parameters.nChannels = 1; // Mono input
    parameters.firstChannel = 0;

    try {
        // Open the audio stream
        audio.openStream(nullptr, &parameters, RTAUDIO_FLOAT32, SAMPLE_RATE, &BUFFER_FRAMES, &audioCallback, nullptr);
        audio.startStream();
    } catch (RtAudioErrorType &e) {
        std::cerr << "Error opening audio stream: " << e << std::endl;
        exit(1);
    }
}

// Reshape function
void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Adjust the orthographic projection to fit the grid
    glOrtho(-GRID_SPAN / 2, GRID_SPAN / 2, -GRID_SPAN / 2, GRID_SPAN / 2, -GRID_SPAN / 2, GRID_SPAN / 2);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Keyboard function
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'q': // Quit
            try {
                audio.stopStream();
            } catch (RtAudioErrorType &e) {
                std::cerr << "Error stopping audio stream: " << e << std::endl;
            }

            if (audio.isStreamOpen()) {
                audio.closeStream();
            }
            exit(0);
            break;
    }
    glutPostRedisplay(); // Request redraw
}

// Mouse function
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
            lastX = x;
            lastY = y;
        } else if (state == GLUT_UP) {
            mouseLeftDown = false;
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseRightDown = true;
            lastX = x;
            lastY = y;
        } else if (state == GLUT_UP) {
            mouseRightDown = false;
        }
    }
}

// Motion function
void motion(int x, int y) {
    if (mouseLeftDown) {
        // Rotate the object
        angleY += (x - lastX);
        angleX += (y - lastY);
        lastX = x;
        lastY = y;
    } else if (mouseRightDown) {
        // Zoom the object
        zoom += (y - lastY) * 0.01; // Adjust zoom based on vertical mouse movement
        lastX = x;
        lastY = y;
    }
    glutPostRedisplay(); // Request redraw
}

// Main function
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Use double buffering
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Audio-Driven NURBS Surface");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard); // Keyboard input
    glutMouseFunc(mouse); // Mouse input
    glutMotionFunc(motion); // Mouse motion
    glutIdleFunc(idle); // Register the idle function for continuous updates

    glutMainLoop(); // Enter the GLUT main loop

    return 0;
}
