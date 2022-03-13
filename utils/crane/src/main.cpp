/**
 * To quickly outline the Plugin API for Crane:
 *
 * You need to have a dynamic library compiled with the following function:
 *
 * CraneContributedCommands *Crane_contributedCommands();
 *
 * Use the `CraneContributedCommands` type to contribute commands to Crane.
 * Loading the library is provided by Crane (AKA Crane Core):
 *
 * load </path/to/contributedLibrary>
 */

int main() {
  return 0;
}
