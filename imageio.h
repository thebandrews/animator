//imageio header file

// This method loads a JPEG or PNG image by filename. The width and height variables are set according to the image that is loaded.
extern unsigned char * loadImage(const char *filename, int &width, int &height);
extern void saveImage(const char * filename, const unsigned char * image, int width, int height, const char * type, int quality); 
