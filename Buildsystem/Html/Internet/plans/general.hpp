// Oliver Kullmann, 24.5.2007 (Swansea)

/*!
  \file Buildsystem/Html/Internet/plans/general.hpp
  \brief Plans for the internet services (web page, etc.)


  \todo Content of the Internet-page:
  <ul>
   <li> Download of the software </li>
   <li> Access to mailing-lists (with instructions). </li>
   <li> Online-documentation (just the documentation-index-page;
   see Buildsystem/Generic/documentation_building/plans/documentation_index.mak.hpp). </li>
   <li> Current external sources to download.
    <ol>
     <li> Package data (see Buildsystem/plans/PackageBuilding.hpp) </li>
    </ol>
   </li>
   <li> A project-page (for example, Heribert Vollmer might have a student
   project about implementing the NP-completeness of SAT (encoding of
   Turing machines)) --- but this perhaps should go to the local system? </li>
  </ul>


  \todo Building the Internet-page:
  <ul>
   <li> A new target "Internet". </li>
   <li> The build-process copies all relevant files to the
   location where the web server accesses the pages. </li>
   <li> The process should be fully automatic, including
   updating the packages etc. --- no alterations by hand
   should be necessary! </li>
  </ul>


  \todo Domain name
  <ul>
   <li> Do we need e-mail-forwarding? </li>
   <li> How to host ok-sat-library.org on our machines DONE (set html-forwarding
   to the local address in Swansea, using the "cloak" method) </li>
   <li> DONE (OK acquired "ok-sat-library.org" for 10 years; forgot to add the
   "www.", but so well --- it would also be fake)
   It sems best to acquire a domain name like
    <ol>
     <li> www. oklibrary.edu </li>
     <li> www. oklibrary.org </li>
    </ol>
   </li>
   <li> How to become owner of such a domain name? DONE </li>
  </ul>

*/

