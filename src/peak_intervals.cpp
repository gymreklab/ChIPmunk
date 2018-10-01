#include "src/common.h"
#include "src/peak_intervals.h"

PeakIntervals::PeakIntervals(const std::string peakfile, const std::string peakfileType,
        const std::string bamfile, const std::int32_t count_colidx) {
  if (!LoadPeaks(peakfile, peakfileType, bamfile, count_colidx)) {
    PrintMessageDieOnError("Error loading peaks from " + peakfile, M_ERROR);
  }
}

PeakIntervals::~PeakIntervals() {}

/*
  Inputs:
  - string peakfile: filename of peaks. tab-separated with chrom, start, end, score

  Outputs:
  - bool: return true if successful, false if error loading peaks

  This function puts the peaks into a searachable data structure
 */
bool PeakIntervals::LoadPeaks(const std::string peakfile, const std::string peakfileType,
        const std::string bamfile, const std::int32_t count_colidx) {
  PeakLoader peakloader(peakfile, peakfileType, bamfile, count_colidx);

  std::vector<Fragment> peaks;
  bool dataLoaded = peakloader.Load(peaks);

  if (dataLoaded){
    // calculate the maximum coverage
    max_coverage = 0;
    for (int peakIndex=0; peakIndex<peaks.size(); peakIndex++){
      if (peaks[peakIndex].score > max_coverage){
        max_coverage = peaks[peakIndex].score;
      }
    }

    // calculate Prob(pulled down|bound)
    double total_signals = 0;
    double total_signals_max = 0;
    for (int peakIndex=0; peakIndex<peaks.size(); peakIndex++){
      total_signals_max += (peaks[peakIndex].length * max_coverage);
      total_signals += (peaks[peakIndex].length * peaks[peakIndex].score);
    }
    prob_pd_given_b = total_signals/total_signals_max;

    // convert the vector into a map with keys as chromosome names
    for (int peakIndex=0; peakIndex<peaks.size(); peakIndex++){
      peak_map[peaks[peakIndex].chrom].push_back(peaks[peakIndex]);
    }
  } 
  return dataLoaded;
}


void PeakIntervals::resetSearchScope(const int index){
  peakIndexStart = index;
}

/*
 * Inputs:
 *   - Fragment frag: a fragment
 *   - std::vector<Fragment>: a list of peaks
 *
 * Outputs:
 *   - float: probablity of the input fragment being bound
 * */
float PeakIntervals::SearchList(const Fragment& frag){
  std::vector<float> probBoundList;
  uint32_t frag_start, frag_end, peak_start, peak_end;
  float overlap;
  /*
  for(int peakIndex=0; peakIndex < peaks.size(); peakIndex++){
    if (frag.chrom == peaks[peakIndex].chrom){
      frag_start = frag.start;
      frag_end = frag.start+frag.length;
      peak_start = peaks[peakIndex].start;
      peak_end = peaks[peakIndex].start+peaks[peakIndex].length;
      if (peak_start < frag_start){
        if (peak_end > frag_start){
            if (peak_end > frag_end){
              probBoundList.push_back(peaks[peakIndex].score/max_coverage);
            }else{
              overlap = (float)(peak_end-frag_start) / (float)(peak_end-peak_start);
              probBoundList.push_back(overlap*peaks[peakIndex].score/max_coverage);
            }
        }else{
            overlap = 0;
        }
      }else if (peak_start < frag_end){
        overlap = (float)( std::min(peak_end,frag_end) - peak_start)/(float)(peak_end-peak_start);
        probBoundList.push_back(overlap*peaks[peakIndex].score/max_coverage);
      }else{
        overlap = 0;
      }
    }
  }*/
  std::vector<Fragment> & peaks = peak_map[frag.chrom];
  for(int peakIndex=peakIndexStart; peakIndex < peaks.size(); peakIndex++){
    if (frag.chrom == peaks[peakIndex].chrom){
      frag_start = frag.start;
      frag_end = frag.start+frag.length;
      peak_start = peaks[peakIndex].start;
      peak_end = peaks[peakIndex].start+peaks[peakIndex].length;
      if (peak_start < frag_start){
        if (peak_end > frag_start){
            if (peak_end > frag_end){
              probBoundList.push_back(peaks[peakIndex].score/max_coverage);
            }else{
              overlap = (float)(peak_end-frag_start) / (float)(peak_end-peak_start);
              probBoundList.push_back(overlap*peaks[peakIndex].score/max_coverage);
            }
        }else{
            overlap = 0;
            peakIndexStart += 1;
        }
      }else if (peak_start < frag_end){
        overlap = (float)( std::min(peak_end,frag_end) - peak_start)/(float)(peak_end-peak_start);
        probBoundList.push_back(overlap*peaks[peakIndex].score/max_coverage);
      }else{
        overlap = 0;
        break;
      }
    }else{
        std::cerr << "****** ERROR: Unexpected errors in PeakInterval/SearchList ******" << std::endl;
        std::exit(1);
    }
  }
  
  // suppose probBoundList = [a, b, c]
  // result = 1- (1-a)*(1-b)*(1-c)
  if (probBoundList.size() > 0){
    //std::cout<<"found"<<std::endl;
    float probBound = 1;
    for (int peakIndex=0; peakIndex < probBoundList.size(); peakIndex++){
       probBound *= (1-probBoundList[peakIndex]);
    }
    probBound = 1-probBound;
    return probBound;
  }else{
    return 0;
  }
}

/*
  Inputs:
  - Fragment frag: has chrom, start, and length of a fragment

  Outputs:
  - float: probability that the fragment is bound

  If the fragment doesn't overlap a peak, return 0
  If it overlaps one ore more peak, return the max score across all peaks
 */
float PeakIntervals::GetOverlap(const Fragment& frag) {
  //std::cout<<frag.start<< " "<<frag.length<<std::endl;
  float score = SearchList(frag);
  //std::cout<<score<<std::endl;
  return score;
  
}
