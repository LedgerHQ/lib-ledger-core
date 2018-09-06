package com.ledger.java;

import java.util.logging.Level;
import java.util.logging.Logger;
/**
 *Class representing a printer of errors, warnings, infos ... (at runtime)
 *Printed logs are stored in path set by PathResolver::resolveLogFilePath
 */
public class LogPrinterImpl extends co.ledger.core.LogPrinter {
    private Logger logger;
    private ExecutionContextImpl executionContext;
    public LogPrinterImpl() {
        this.logger = Logger.getLogger("ledger.reactnative");
        //this.logger = new Logger("ledger.reactnative", null);
        this.executionContext = new ExecutionContextImpl("__logger__");
    }
    /**
     *Print different encountered errors
     *@param message, string
     */
    public void printError(String message) {
        this.logger.log(Level.SEVERE, message);
    }

    /**
     *Print useful information messages
     *@param message, string
     */
    public void printInfo(String message) {
        this.logger.log(Level.INFO, message);
    }

    /**
     *Print debug messages
     *@param message string
     */
    public void printDebug(String message) {
        this.logger.log(Level.FINE, message);
    }

    /**
     *Print warning messages
     *@param message, string
     */
    public void printWarning(String message) {
        this.logger.log(Level.WARNING, message);
    }

    /**
     *Print messages from APDU comand interpretation loop
     *@param message, string
     */
    public void printApdu(String message) {
        this.logger.log(Level.INFO, message);
    }

    /**
     *Print critical errors causing a core dump or error from which recovery is impossible
     *@param message, string
     */
    public void printCriticalError(String message) {
        this.logger.log(Level.SEVERE, message);
    }

    /**
     *Get context in which printer is executed (print)
     *@return ExecutionContext object
     */
    public co.ledger.core.ExecutionContext getContext() {
    	return this.executionContext;
    }
}
