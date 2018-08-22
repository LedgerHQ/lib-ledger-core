package co.ledger.core;

/**
 * Module used to resolve file paths. libledger-core has its own iternal representation of the file system that may not
 * be adapted for the runtime platform. All path given to the PathResolver are absolute.
 */
public class PathResolverImpl extends PathResolver {
    /**
     * Resolves the path for a SQLite database file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolveDatabasePath(String path) {

    }

    /**
     * Resolves the path of a single log file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolveLogFilePath(String path) {

    }

    /**
     * Resolves the path for a json file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolvePreferencesPath(String path) {

    }
}
