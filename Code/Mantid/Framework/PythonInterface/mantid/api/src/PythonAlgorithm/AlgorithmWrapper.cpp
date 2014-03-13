#include "MantidPythonInterface/api/PythonAlgorithm/AlgorithmWrapper.h"
#include "MantidPythonInterface/kernel/Environment/WrapperHelpers.h"
#include "MantidPythonInterface/kernel/Environment/CallMethod.h"
#include "MantidPythonInterface/kernel/Environment/Threading.h"

#include <boost/python/class.hpp>
#include <boost/python/dict.hpp>

//-----------------------------------------------------------------------------
// AlgorithmWrapper definition
//-----------------------------------------------------------------------------
namespace Mantid
{
  namespace PythonInterface
  {
    using namespace boost::python;
    using Environment::CallMethod0;

    /**
     * Construct the "wrapper" and stores the reference to the PyObject
     * @param self A reference to the calling Python object
     */
    AlgorithmWrapper::AlgorithmWrapper(PyObject* self)
      : PythonAlgorithm(), m_self(self), m_isRunningObj(NULL)
    {
      // Cache the isRunning call to save the lookup each time it is called
      // as it is most likely called in a loop

      // If the derived class type has isRunning then us that.
      // A standard PyObject_HasAttr will check the whole inheritance
      // hierarchy and always return true because IAlgorithm::isRunning is present.
      // We just want to look at the Python class
      if(Environment::typeHasAttribute(self, "isRunning"))
        m_isRunningObj = PyObject_GetAttrString(self, "isRunning");
    }

    /**
     * Returns the name of the algorithm. This cannot be overridden in Python.
     */
    const std::string AlgorithmWrapper::name() const
    {
      return std::string(getSelf()->ob_type->tp_name);
    }

    /**
     * Returns the version of the algorithm. If not overridden
     * it returns 1
     */
    int AlgorithmWrapper::version() const
    {
      return CallMethod0<int>::dispatchWithDefaultReturn(getSelf(), "version", defaultVersion());
    }

    int AlgorithmWrapper::defaultVersion() const
    {
      return 1;
    }

    /**
     * Returns the category of the algorithm. If not overridden
     * it returns "PythonAlgorithm"
     */
    const std::string AlgorithmWrapper::category() const
    {
      return CallMethod0<std::string>::dispatchWithDefaultReturn(getSelf(), "category", defaultCategory());
    }

    /**
     * A default category, chosen if there is no override
     * @returns A default category
     */
    std::string AlgorithmWrapper::defaultCategory() const
    {
      return "PythonAlgorithms";
    }

    /**
     * @return True if the algorithm is considered to be running
     */
    bool AlgorithmWrapper::isRunning() const
    {
      if(!m_isRunningObj)
      {
        return Algorithm::isRunning();
      }
      else
      {
        Environment::GlobalInterpreterLock gil;
        PyObject *result = PyObject_CallObject(m_isRunningObj, NULL);
        if(PyErr_Occurred()) Environment::throwRuntimeError(true);
        if(PyBool_Check(result)) return PyInt_AsLong(result);
        else throw std::runtime_error("PythonAlgorithm.isRunning - Expected bool return type.");
      }
    }

    /**
     */
    void AlgorithmWrapper::cancel()
    {
      std::cerr << "in c++\n";
      // No real need for eye on performance here. Use standard methods
      if(Environment::typeHasAttribute(getSelf(), "cancel"))
      {
        Environment::GlobalInterpreterLock gil;
        CallMethod0<void>::dispatchWithException(getSelf(), "cancel");
      }
      else Algorithm::cancel();
    }

    /**
     * @copydoc Mantid::API::Algorithm::validateInputs
     */
    std::map<std::string, std::string> AlgorithmWrapper::validateInputs()
    {
      // variables that are needed further down
      boost::python::dict resultDict;
      std::map<std::string, std::string> resultMap;

      // this is a modified version of CallMethod0::dispatchWithDefaultReturn
      Environment::GlobalInterpreterLock gil;
      if(Environment::typeHasAttribute(getSelf(), "validateInputs"))
      {
        try
        {
          resultDict = boost::python::call_method<boost::python::dict>(getSelf(),"validateInputs");

          if (!bool(resultDict))
            return resultMap;
        }
        catch(boost::python::error_already_set&)
        {
          Environment::throwRuntimeError();
        }
      }

      // convert to a map<string,string>
      boost::python::list keys = resultDict.keys();
      size_t numItems = boost::python::len(keys);
      for (size_t i = 0; i < numItems; ++i)
      {
        boost::python::object value = resultDict[keys[i]];
        if (value)
        {
          try
          {
            std::string key = boost::python::extract<std::string>(keys[i]);
            std::string value = boost::python::extract<std::string>(resultDict[keys[i]]);
            resultMap[key] = value;
          }
          catch(boost::python::error_already_set &)
          {
            this->getLogger().error() << "In validateInputs(self): Invalid type for key/value pair "
                                      << "detected in dict.\n"
                                      << "All keys and values must be strings\n";
          }
        }
      }
      return resultMap;
    }

    /**
     * Private init for this algorithm. Expected to be
     * overridden in the subclass by a function named PyInit
     */
    void AlgorithmWrapper::init()
    {
      CallMethod0<void>::dispatchWithException(getSelf(), "PyInit");
    }

    /**
     * Private exec for this algorithm. Expected to be
     * overridden in the subclass by a function named PyExec
     */
    void AlgorithmWrapper::exec()
    {

      CallMethod0<void>::dispatchWithException(getSelf(), "PyExec");
    }

  }
}
