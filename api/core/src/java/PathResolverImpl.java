package com.ledger.java;

import android.content.Context;

import java.io.File;

/**
 * Module used to resolve file paths. libledger-core has its own iternal representation of the file system that may not
 * be adapted for the runtime platform. All path given to the PathResolver are absolute.
 */
public class PathResolverImpl extends co.ledger.core.PathResolver {
    public PathResolverImpl() {
    }
    /**
     * Resolves the path for a SQLite database file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolveDatabasePath(String path) {
		File file = context.getFilesDir();
        return "";
    }

    /**
     * Resolves the path of a single log file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolveLogFilePath(String path) {
		return "";
    }

    /**
     * Resolves the path for a json file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolvePreferencesPath(String path) {
		return "";
    }
}
