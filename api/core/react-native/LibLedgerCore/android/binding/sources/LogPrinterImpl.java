package co.ledger.core;

/**
 *Class representing a printer of errors, warnings, infos ... (at runtime)
 *Printed logs are stored in path set by PathResolver::resolveLogFilePath
 */
public class LogPrinterImpl extends LogPrinter {
    /**
     *Print different encountered errors
     *@param message, string
     */
    public void printError(String message) {

    }

    /**
     *Print useful information messages
     *@param message, string
     */
    public void printInfo(String message) {

    }

    /**
     *Print debug messages
     *@param message string
     */
    public void printDebug(String message) {

    }

    /**
     *Print warning messages
     *@param message, string
     */
    public void printWarning(String message) {

    }

    /**
     *Print messages from APDU comand interpretation loop
     *@param message, string
     */
    public void printApdu(String message) {

    }

    /**
     *Print critical errors causing a core dump or error from which recovery is impossible
     *@param message, string
     */
    public void printCriticalError(String message) {

    }

    /**
     *Get context in which printer is executed (print)
     *@return ExecutionContext object
     */
    public ExecutionContext getContext() {
    	return null;
    }
}
