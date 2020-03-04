/** Load a SEQ file, a text file in PETSCII format.
 * Bear in mind this is different from screen codes!
 * @param filename The filename of the RLE packed SEQ file
 * @param size The size of the returned data
 * @return The file contents
 */
unsigned char* seq_load (char filename[], unsigned int* size);
