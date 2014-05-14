try:
    import mantidvsipython as mvp

    def create_window(window_class, *setups):
        """
        Function to create the requested widget.

        Parameters:
        ----------

        setups :: Functions (as strings) that need to be called on the widget 
                  before showing.
        """
    	from PyQt4 import QtCore
        from PyQt4 import QtGui
        import sys

	app_created = False
    	app = QtCore.QCoreApplication.instance()
	if app is None:
	    app = QtGui.QApplication(sys.argv)
            app_created = True
        app.references = set()
        window = window_class()
        for setup in setups:
            getattr(window, setup)()
        app.references.add(window)
        window.show()
        if app_created:
            app.exec_()
        return window

    def show_vsi():
        widget = create_window(mvp.Mantid.Vates.SimpleGui.MdViewerWidget,
                               "setupPluginMode")
        return widget

except ImportError:
    print "MantidVsi module is not available!"
    
