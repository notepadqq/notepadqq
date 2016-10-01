#ifndef AUTOSAVE_H
#define AUTOSAVE_H

namespace Autosave {

/**
 * @brief restoreFromAutosave Reads the autosave sessions and recreates
 *        all windows and their tabs.
 */
void restoreFromAutosave();

/**
 * @brief enableAutosave Starts periodic autosaving.
 */
void enableAutosave();

/**
 * @brief disableAutosave Stops periodic autosaving.
 */
void disableAutosave();

} // namespace Autosave


#endif // AUTOSAVE_H
