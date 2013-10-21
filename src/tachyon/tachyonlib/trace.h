/* 
 * trace.h - This file contains the declarations for the main tracing calls.
 *
 *   $Id: trace.h,v 1.33 2011/02/05 08:10:11 johns Exp $
 */

#include "threads.h"

typedef struct {
  int tid;                    /**< worker thread index            */
  int nthr;                   /**< total number of worker threads */
  scenedef * scene;           /**< scene handle                   */
  unsigned long * local_mbox; /**< grid acceleration mailbox structure */
  unsigned long serialno;     /**< ray mailbox test serial number */
  int startx;                 /**< starting X pixel index         */
  int stopx;                  /**< ending X pixel index           */
  int xinc;                   /**< X pixel stride                 */
  int starty;                 /**< starting Y pixel index         */
  int stopy;                  /**< ending Y pixel index           */
  int yinc;                   /**< Y pixel stride                 */
  rt_barrier_t * runbar;      /**< Sleeping thread pool barrier   */
} thr_parms;

color trace(ray *);
void * thread_trace(thr_parms *); 

