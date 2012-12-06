/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2012, Individual contributors, see AUTHORS file        |
   | Copyright (c) 2005-2012, MAPIR group, University of Malaga                |
   | Copyright (c) 2012, University of Almeria                                 |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   +---------------------------------------------------------------------------+ */
#ifndef CMetricMapBuilder_H
#define CMetricMapBuilder_H

#include <mrpt/utils/CSerializable.h>
#include <mrpt/utils/CListOfClasses.h>
#include <mrpt/utils/CDebugOutputCapable.h>
#include <mrpt/synch.h>
#include <mrpt/slam/CMultiMetricMap.h>
#include <mrpt/slam/CSensoryFrame.h>
#include <mrpt/slam/CSimpleMap.h>
#include <mrpt/poses/CPose3DPDF.h>
#include <mrpt/slam/CActionCollection.h>

#include <mrpt/slam/link_pragmas.h>

namespace mrpt
{
namespace slam
{
	/** @defgroup metric_slam_grp Metric SLAM algorithms
	  * \ingroup mrpt_slam_grp */

	/** This virtual class is the base for SLAM implementations. See derived classes for more information.
	 *
	 * \sa CMetricMap  \ingroup metric_slam_grp
	 */
	class SLAM_IMPEXP CMetricMapBuilder : public mrpt::utils::CDebugOutputCapable
	{
	protected:
		/** Critical zones
		  */
		synch::CCriticalSection	critZoneChangingMap;

		/** Enter critical section for map updating:
		  */
		inline void  enterCriticalSection() { critZoneChangingMap.enter(); }

		/** Leave critical section for map updating:
		  */
		inline void  leaveCriticalSection() { critZoneChangingMap.leave(); }

	public:
		CMetricMapBuilder();           //!< Constructor
		virtual ~CMetricMapBuilder( ); //!< Destructor.

		// ---------------------------------------------------------------------
		/** @name Pure virtual methods to implement in any particular SLAM algorithm
		    @{ */

		/** Initialize the method, starting with a known location PDF "x0"(if supplied, set to NULL to left unmodified) and a given fixed, past map. */
		virtual void  initialize(
				const CSimpleMap &initialMap = CSimpleMap(),
				CPosePDF					*x0 = NULL
				) = 0;

		/** Returns a copy of the current best pose estimation as a pose PDF. */
		virtual CPose3DPDFPtr  getCurrentPoseEstimation() const = 0;

		/** Process a new action and observations pair to update this map: See the description of the class at the top of this page to see a more complete description.
		 *  \param action The estimation of the incremental pose change in the robot pose.
		 *	\param observations The set of observations that robot senses at the new pose.
		 */
		virtual void  processActionObservation( CActionCollection &action,CSensoryFrame	&observations ) = 0;

		/** Fills "out_map" with the set of "poses"-"sensory-frames", thus the so far built map. */
		virtual void  getCurrentlyBuiltMap(CSimpleMap &out_map) const = 0;

		/** Returns just how many sensory-frames are stored in the currently build map. */
		virtual unsigned int  getCurrentlyBuiltMapSize() = 0;

		/** Returns the map built so far. NOTE that for efficiency a pointer to the internal object is passed, DO NOT delete nor modify the object in any way, if desired, make a copy of ir with "duplicate()". */
		virtual CMultiMetricMap*   getCurrentlyBuiltMetricMap() = 0;

		/** A useful method for debugging: the current map (and/or poses) estimation is dumped to an image file.
		  * \param file The output file name
		  * \param formatEMF_BMP Output format = true:EMF, false:BMP
		  */
		virtual void  saveCurrentEstimationToImage(const std::string &file, bool formatEMF_BMP = true) = 0;

		/** @} */ 
		// ---------------------------------------------------------------------

		/** Clear all elements of the maps, and reset localization to (0,0,0deg). */
		void  clear();

		/** Enables or disables the map updating (default state is enabled) */
		void  enableMapUpdating( bool enable )
		{
			options.enableMapUpdating = enable;
		}

		/** Load map (mrpt::slam::CSimpleMap) from a ".simplemap" file */
		void  loadCurrentMapFromFile(const std::string &fileName);

		/** Save map (mrpt::slam::CSimpleMap) to a ".simplemap" file. */
		void  saveCurrentMapToFile(const std::string &fileName, bool compressGZ=true) const;

		/** Options for the algorithm */
		struct SLAM_IMPEXP TOptions
		{
			TOptions() : verbose(true),
						 enableMapUpdating(true),
						 debugForceInsertion(false),
						 alwaysInsertByClass()
			{
			}

			bool	verbose;             //!< If true shows debug information in the console, default is true.
			bool	enableMapUpdating;   //!< Enable map updating, default is true.
			bool	debugForceInsertion; //!< Always insert into map. Default is false: detect if necesary.

			/** A list of observation classes (derived from mrpt::slam::CObservation) which will be always inserted in the map, disregarding the minimum insertion distances).
			  *  Default: Empty. How to insert classes:
			  *   \code
			  *     alwaysInserByClass.insert(CLASS_ID(CObservationImage));
			  *   \endcode
			  * \sa mrpt::utils::CListOfClasses
			  */
			mrpt::utils::CListOfClasses		alwaysInsertByClass;

		};

		TOptions options;

		public: 
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW 

	}; // End of class def.

	} // End of namespace
} // End of namespace

#endif
