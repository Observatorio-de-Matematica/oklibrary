// Oliver Kullmann, 31.8.2007 (Swansea)

/*!
  \file Buildsystem/ExternalSources/SpecialBuilds/plans/Apache.hpp
  \brief Plans regarding installation of Apache


  \todo Building Apache
  <ul>
   <li> Building apache on cs-wsok (for testing):
    <ol>
     <li> Simple manual installation:
     \verbatim
ExternalSources> mkdir installations
xternalSources> mkdir installations/Apache
ExternalSources> cd installations/Apache/
ExternalSources/installations/Apache> tar -xjf ../../sources/Apache/httpd-2.2.4.tar.bz2
ExternalSources/installations/Apache> cd httpd-2.2.4/
ExternalSources/installations/Apache/httpd-2.2.4> ./configure
ExternalSources/installations/Apache/httpd-2.2.4> make
     \endverbatim
     result in the error
     \verbatim
/usr/lib/libexpat.so: could not read symbols: File in wrong format
collect2: ld returned 1 exit status
make[3]: *** [libaprutil-1.la] Error 1
     \endverbatim
     </li>
     <li> Perhaps apr and apr-util has to be installed manually?
     After changing to directory  httpd-2.2.4:
     \verbatim
ExternalSources/installations/Apache/httpd-2.2.4> cd srclib/apr
ExternalSources/installations/Apache/httpd-2.2.4/srclib/apr> ./configure --prefix=/usr/local/apr-httpd/
ExternalSources/installations/Apache/httpd-2.2.4/srclib/apr> make
ExternalSources/installations/Apache/httpd-2.2.4/srclib/apr> su
ExternalSources/installations/Apache/httpd-2.2.4/srclib/apr # make install
ExternalSources/installations/Apache/httpd-2.2.4/srclib/apr # exit
ExternalSources/installations/Apache/httpd-2.2.4/srclib/apr> cd ../apr-util
ExternalSources/installations/Apache/httpd-2.2.4/srclib/apr-util> ./configure --prefix=/usr/local/apr-util-httpd/ --with-apr=/usr/local/apr-httpd/
ExternalSources/installations/Apache/httpd-2.2.4/srclib/apr-util> make
     \endverbatim
     yields the same error:
     \verbatim
/usr/lib/libexpat.so: could not read symbols: File in wrong format
collect2: ld returned 1 exit status
make[1]: *** [libaprutil-1.la] Error 1
     \endverbatim
     So there is something wrong with the installation of expat. </li>
     <li> So let's install expat first.
     \verbatim
ExternalSources/installations> mkdir Expat
ExternalSources/installations> tar -xzf ../sources/Expat/expat-2.0.1.tar.gz
ExternalSources/installations> cd expat-2.0.1/
ExternalSources/installations/expat-2.0.1> ./configure
ExternalSources/installations/expat-2.0.1> make
ExternalSources/installations/expat-2.0.1> sudo make install
     \endverbatim
     Libraries have been installed in /usr/local/lib.
     Removal of .a, .la, and .so-libraries in /usr/lib.
     </li>
     <li> Repeating the Apache installation now seemed to work. Completed with
     \verbatim
ExternalSources/installations/Apache/httpd-2.2.4> sudo make install
     \endverbatim
     </li>
     <li> Tested with
     \verbatim
     # /usr/local/apache2/bin/apachectl -k start
     \endverbatim
     and then accessign http://localhost/index.html, which works (and hopefully
     really needs Apache). After stoping
     \verbatim
     # /usr/local/apache2/bin/apachectl -k stop
     \endverbatim
     the page http://localhost/index.html is no longer accessible (so it
     seems to work). </li>
    </ol>
   </li>
   <li> What do we need to configure? </li>
   <li> The configuration file should be placed under Buildsystem/Configuration. </li>
   <li> It seems a special system-user "httpd" should be created? </li>
   <li> But perhaps we should now first try to install mailman? </li>
  </ul>

*/

