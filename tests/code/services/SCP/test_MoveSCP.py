
import os
import subprocess
import unittest

from code.testbase import TestBase

class TestMoveSCP(TestBase):
    
    #################################################
    # Nominal test case: Execute Move
    #################################################
    def test_run_movescu(self):
        try:
            pathjoin = os.path.join(self._output_directory,
                                    "MR.2.16.756.5.5.100.3611280983.20092.1364462458.1.0")
            self.assertEqual(os.path.isfile(pathjoin), False)
            
            # Send a Move request
            subproc = subprocess.Popen(["movescu", "-aet", "LOCAL", "-aec",
                                        "REMOTE", "-aem", "LOCAL", "+P", self._scu_port,
                                        "-P", "-k", "0010,0010=Doe^Jane",
                                        "-k", " 0008,0052=PATIENT",
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertEqual(err, "")
            
            # Check result file
            self.assertEqual(os.path.isfile(pathjoin), True)
            
            # remove file
            os.remove(pathjoin)
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
    
    #################################################
    # Nominal test case: Not Allowed
    #################################################
    def test_move_not_allowed(self):
        # Remove authorization from database
        thread_spec = subprocess.Popen(self._create_specific_auth, shell=True)
        thread_spec.wait()
        
        try:
            pathjoin = os.path.join(self._output_directory,
                                    "MR.2.16.756.5.5.100.3611280983.20092.1364462458.1.0")
            self.assertEqual(os.path.isfile(pathjoin), False)
            
            # Send a Move request
            subproc = subprocess.Popen(["movescu", "-aet", "LOCAL", "-aec",
                                        "REMOTE", "-aem", "LOCAL", "+P", self._scu_port,
                                        "-P", "-k", "0010,0010=Doe^Jane",
                                        "-k", " 0008,0052=PATIENT",
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertEqual(err, "")
            
            # Check result file (no file)
            self.assertEqual(os.path.isfile(pathjoin), False)
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
        
    #################################################
    # Error test case: No QueryRetrieveLevel
    #################################################
    def test_move_no_queryretrievelevel(self):
        try:
            # Send a Move request
            subproc = subprocess.Popen(["movescu", "-aet", "LOCAL", "-aec",
                                        "REMOTE", "-aem", "LOCAL", "+P", self._scu_port,
                                        "-P", "-k", "0010,0010=Doe^Jane",
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
        
    #################################################
    # Error test case: No Authentication
    #################################################
    def test_move_not_authenticate(self):
        # Remove authorization from database
        thread_remove = subprocess.Popen(self._remove_authorization, shell=True)
        thread_remove.wait()
        
        try:
            # Send a Move request
            subproc = subprocess.Popen(["movescu", "-aet", "LOCAL", "-aec",
                                        "REMOTE", "-aem", "LOCAL", "+P", self._scu_port,
                                        "-P", "-k", "0010,0010=Doe^Jane",
                                        "-k", " 0008,0052=PATIENT",
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()
