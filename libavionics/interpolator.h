#ifndef __INTERPOLATOR_H__
#define __INTERPOLATOR_H__

#include <vector>
#include <algorithm>

/// Abstract interpolator, defines N-dimensional interpolation function, stores grid and M-dimensional function, its manipulations,
/// defines interpolating type (linear, quadratic, cubic spline)
template <typename T> class Interpolator {
public:
	/// Interpolation types
	enum eInterpType { type_LINEAR = 0, type_QUADRATIC = 1, type_QUBIC = 2 };

	/// New empty-data interpolator, generates specific ID
	Interpolator(const eInterpType& inType = type_LINEAR) {
		mIDGenerator++;
		mID = mIDGenerator;
		mInterpolationType = inType;
	}

	virtual ~Interpolator() {}

public:	

	/// Predicate for searching in grid
	static bool lowerBoundCompare(const T& inArg1, const T& inArg2) {
		return inArg1 <= inArg2;
	}

private:

	/// Gets all directions gradients from current point
	std::vector<T> getGradients(const std::size_t& inFunctionDim, const std::vector<std::size_t>& inPos) const {
		std::size_t gradient_index = 0;
		std::size_t posMultiplier = 1;
		for (std::size_t i = 0; i < mGridDelimiters.size(); i++) {
			gradient_index += posMultiplier * inPos[i];
			posMultiplier *= mGridDelimiters[i];
		}

		std::vector<T> gradients;
		for (std::size_t i = 0; i < mGridDelimiters.size(); i++) {
			gradients.push_back(mGradients[inFunctionDim][mGridDelimiters.size() * gradient_index + i]);
		}

		return gradients;
	}

	/// Helper for solving linear system
	static void solveLinearSystemExchange(std::vector<std::vector<T> >& inCoeffMatrix, const std::size_t& inFrom, const std::size_t& inTo) {
		for (std::size_t i = 0; i < inCoeffMatrix.size(); i++){
			inCoeffMatrix[inFrom][i] = inCoeffMatrix[inFrom][i] + inCoeffMatrix[inTo][i];
			inCoeffMatrix[inTo][i] = inCoeffMatrix[inFrom][i] - inCoeffMatrix[inTo][i];
			inCoeffMatrix[inFrom][i] = inCoeffMatrix[inFrom][i] - inCoeffMatrix[inTo][i];
		}
	}

	/// Linear system solving
	static std::vector<double> solveLinearSystem(std::vector<std::vector<T> >& inCoeffMatrix, std::vector<T>& inRightVector) {
		if (inCoeffMatrix.size() != inCoeffMatrix[0].size()) {
			return std::vector<double>(1, -1);
		}

		std::vector<double> result(inCoeffMatrix.size());

		for (std::size_t k = 0; k < inCoeffMatrix.size() - 1; k++) {
			for (std::size_t m = k + 1; m < inCoeffMatrix.size(); m++) {
				if (inCoeffMatrix[m][m] == 0) {
					std::size_t temp_index = 0;
					for (std::size_t i = m + 1; i < inCoeffMatrix.size(); i++){
						if (inCoeffMatrix[i][i] != 0) {
							temp_index = i;
						}
					}
					solveLinearSystemExchange(inCoeffMatrix, m, temp_index);
				}

				double coefficent = inCoeffMatrix[m][k] / inCoeffMatrix[0][k];

				inRightVector[m] = inRightVector[m] - inRightVector[0] * coefficent;
				for (std::size_t z = 0; z < inCoeffMatrix.size(); z++){
					inCoeffMatrix[m][z] = inCoeffMatrix[m][k] - inCoeffMatrix[0][k] * coefficent;
				}

			}
		}
		
		for (std::size_t m = inCoeffMatrix.size() - 1;; m--) {
			double tempSum = 0.0f;
			for (std::size_t i = inCoeffMatrix.size() - 1; i > m; i--) {
				tempSum += result[i] * inCoeffMatrix[m][i];
			}
			result[m] = (inRightVector[m] - tempSum) / inCoeffMatrix[m][m];
			if (!m) {
				break;
			}
		}

		return result;
	}

public:

	/// Sets grid delimiters, it defines grid point counts in all directions  
	void setGridDelimiters(const std::vector<std::size_t>& inGridDelimiters) {
		mGridDelimiters = inGridDelimiters;
	}

	/// Sets function values. Note: function values must be stored in 1-dimensional vector, independently of N  
	void addFunction(const std::vector<T>& inFunction) {
		mFunctions.push_back(inFunction);
		mGradients.resize(mFunctions.size());
	}

	/// Gets function value from next to current point in specific direction
	/// Note: when parameter inDirection > N - 1, returns function value in current point
	double getFunctionValue(const std::size_t& inFunctionDim, const std::vector<std::size_t>& inPos, const std::size_t& inDirection) const {
		std::size_t posIndex = 0;
		std::size_t posMultiplier = 1;
		for (std::size_t i = 0; i < inPos.size(); i++) {
			posIndex += posMultiplier * inPos[i];
			if (i == inDirection) {
				posIndex += posMultiplier;
			}
			posMultiplier *= mGridDelimiters[i];
		}

		return mFunctions[inFunctionDim][posIndex];

	}

	/// Gets grid value from current point in specific direction 
	double getGridValue(const std::vector<std::size_t>& inPos, const std::size_t& inDirection) const {
		std::size_t posIndex = 0;
		for (std::size_t i = 0; i < inPos.size(); i++) {
			if (i == inDirection) {
				posIndex += inPos[i];
				break;
			} else {
				posIndex += mGridDelimiters[i];
			}
		}
		return mGrid[posIndex];
	}

	/// Validates the interpolator object. Must be called after all set() functions calls
	bool validate() {
		std::size_t functionValidSize = 1;
		for (std::size_t i = 0; i < mGridDelimiters.size(); i++) {
			functionValidSize *= mGridDelimiters[i];
		}
		mFunctionDimension = mFunctions[0].size() / functionValidSize;
		if (mGridDelimiters.size() > 5) {
			return false;
		}
		for (std::size_t i = 0; i < mGridDelimiters.size(); i++) {
			if (mGridDelimiters[i] < 2) {
				return false;
			}
		}
		for (std::size_t i = 0; i < mFunctions.size(); i++) {
			if (mFunctions[i].size() % functionValidSize) {
				return false;
			}
		}
		return true;
	}

	/// Calculates gradients. Overrides by all interpolators, that inherits from 
	virtual void calculateGradients() = 0;

	/// Sets all grid values
	void setGrid(const std::vector<T>& inGrid) {
		mGrid = inGrid;
	}
	
	/// Gets current interpolator object ID
	int getID() const {
		return mID;
	}

	/// N-dimensional interpolation
	std::vector<T> interpolate(const std::vector<T>& inPoint, bool isClosed) const {

		std::vector<T> interp_vector;
		interp_vector.resize(mFunctions.size());

		for (std::size_t m = 0; m < mFunctions.size(); m++) {
			std::vector<std::size_t> indexes;
			std::size_t index_shift = 0;
			std::vector<bool> leftToRangeCondition(mGridDelimiters.size(), false);

			for (std::size_t i = 0; i < mGridDelimiters.size(); i++) {
				std::size_t index = std::distance(mGrid.begin() + index_shift,
					std::lower_bound(mGrid.begin() + index_shift, mGrid.begin() + index_shift + mGridDelimiters[i], inPoint[i], lowerBoundCompare));
				if (index) {
					indexes.push_back(index - 1);
				} else {
					indexes.push_back(index);
					leftToRangeCondition[i] = true;
				}
				index_shift += mGridDelimiters[i];
			}

			T interp_result = getFunctionValue(m, indexes, mGridDelimiters.size());
			std::vector<T> interp_gradients = getGradients(m, indexes);

			for (std::size_t i = 0; i < mGridDelimiters.size(); i++) {
				std::size_t cur_last_index = mGridDelimiters[i] - 1;

				if (isClosed && ((indexes[i] > cur_last_index - 1) || leftToRangeCondition[i])) {
					continue;
				} else if (indexes[i] > cur_last_index - 1) {
					std::vector<std::size_t > prevDirectionIndexes = indexes;
					prevDirectionIndexes[i]--;
					std::vector<T> prevDirectionGradients = getGradients(m, prevDirectionIndexes);
					interp_result += prevDirectionGradients[i] * (inPoint[i] - getGridValue(indexes, i));
				} else {
					interp_result += interp_gradients[i] * (inPoint[i] - getGridValue(indexes, i));
				}

			}

			interp_vector[m] = interp_result;

		}

		return interp_vector;
	}

protected:
	int				mID;
	static int			mIDGenerator;
	eInterpType				mInterpolationType;

	bool				mIsValid;
	std::vector<T>				mGrid;
	std::vector<std::size_t>				mGridDelimiters;

	std::vector<std::vector<T> > 				mFunctions;
	std::size_t				mFunctionDimension;
	std::vector<std::vector<T> >				mGradients;
};

// 1D interpolator
template <typename T> class Interpolator1D : public Interpolator<T> {
public:

	using Interpolator<T>::getFunctionValue;
	using Interpolator<T>::getGridValue;

	Interpolator1D() : Interpolator<T>() {}
	virtual ~Interpolator1D() {}

	virtual void calculateGradients() {

		for (std::size_t m = 0; m < this->mFunctions.size(); m++) {

			std::vector<std::size_t> pos(1, 0);
			for (std::size_t i = 0; i < this->mGridDelimiters[0]; i++) {
				std::vector<std::size_t> next_pos = pos;
				next_pos[0]++;
				if (i == this->mGridDelimiters[0] - 1) {
					this->mGradients[m].push_back(getFunctionValue(m, pos, this->mGridDelimiters.size()));
				}
				this->mGradients[m].push_back((getFunctionValue(m, pos, 0) - getFunctionValue(m, pos, this->mGridDelimiters.size())) / (getGridValue(next_pos, 0) - getGridValue(pos, 0)));
				if (i != this->mGridDelimiters[0] - 2) {
					pos[0]++;
				}
			}

		}
	}
};

// 2D interpolator
template <typename T> class Interpolator2D : public Interpolator<T> {
public:
	using Interpolator<T>::getFunctionValue;
	using Interpolator<T>::getGridValue;

	Interpolator2D() : Interpolator<T>() {}
	virtual ~Interpolator2D() {}

	virtual void calculateGradients() {

		for (std::size_t m = 0; m < this->mFunctions.size(); m++) {

			std::vector<std::size_t> pos(2, 0);
			for (std::size_t i = 0; i < this->mGridDelimiters[1]; i++) {
				for (std::size_t j = 0; j < this->mGridDelimiters[0]; j++) {
					for (std::size_t k = 0; k < this->mGridDelimiters.size(); k++) {
						std::vector<std::size_t> next_pos = pos;
						next_pos[k]++;
						if ((k == 1 && i == this->mGridDelimiters[1] - 1) || (k == 0 && j == this->mGridDelimiters[0] - 1)) {
							this->mGradients[m].push_back(getFunctionValue(m, pos, this->mGridDelimiters.size()));
						} else {
							this->mGradients[m].push_back((getFunctionValue(m, pos, k) - getFunctionValue(m, pos, this->mGridDelimiters.size())) / (getGridValue(next_pos, k) - getGridValue(pos, k)));
						}
					}
					pos[0]++;
				}
				pos[1]++;
				pos[0] = 0;
			}

		}
	}
};

// 3D interpolator
template <typename T> class Interpolator3D : public Interpolator<T> {
public:
	using Interpolator<T>::getFunctionValue;
	using Interpolator<T>::getGridValue;

	Interpolator3D() : Interpolator<T>() {}
	virtual ~Interpolator3D() {}

	virtual void calculateGradients() {

		for (std::size_t m = 0; m < this->mFunctions.size(); m++) {

			std::vector<std::size_t> pos(3, 0);
			for (std::size_t l = 0; l < this->mGridDelimiters[2]; l++) {
				for (std::size_t i = 0; i < this->mGridDelimiters[1]; i++) {
					for (std::size_t j = 0; j < this->mGridDelimiters[0]; j++) {
						for (std::size_t k = 0; k < this->mGridDelimiters.size(); k++) {
							std::vector<std::size_t> next_pos = pos;
							next_pos[k]++;
							if ((k == 1 && i == this->mGridDelimiters[1] - 1) || (k == 0 && j == this->mGridDelimiters[0] - 1) || (k == 2 && l == this->mGridDelimiters[2] - 1)) {
								this->mGradients[m].push_back(getFunctionValue(m, pos, this->mGridDelimiters.size()));
							} else {
								this->mGradients[m].push_back((getFunctionValue(m, pos, k) - getFunctionValue(m, pos, this->mGridDelimiters.size())) / (getGridValue(next_pos, k) - getGridValue(pos, k)));
							}
						}
						pos[0]++;
					}
					pos[1]++;
					pos[0] = 0;
				}
				pos[2]++;
				pos[1] = 0;
				pos[0] = 0;
			}

		}
	}
};

// 4D interpolator
template <typename T> class Interpolator4D : public Interpolator<T> {
public:
	using Interpolator<T>::getFunctionValue;
	using Interpolator<T>::getGridValue;

	Interpolator4D() : Interpolator<T>() {}
	virtual ~Interpolator4D() {}

	virtual void calculateGradients() {

		for (std::size_t m = 0; m < this->mFunctions.size(); m++) {

			std::vector<std::size_t> pos(4, 0);
			for (std::size_t m = 0; m < this->mGridDelimiters[3]; m++) {
				for (std::size_t l = 0; l < this->mGridDelimiters[2]; l++) {
					for (std::size_t i = 0; i < this->mGridDelimiters[1]; i++) {
						for (std::size_t j = 0; j < this->mGridDelimiters[0]; j++) {
							for (std::size_t k = 0; k < this->mGridDelimiters.size(); k++) {
								std::vector<std::size_t> next_pos = pos;
								next_pos[k]++;
								if ((k == 3 && m == this->mGridDelimiters[3] - 1) || (k == 2 && l == this->mGridDelimiters[2] - 1) ||
									(k == 1 && i == this->mGridDelimiters[1] - 1) || (k == 0 && j == this->mGridDelimiters[0] - 1)) {
									this->mGradients[m].push_back(getFunctionValue(m, pos, this->mGridDelimiters.size()));
								} else {
									this->mGradients[m].push_back((getFunctionValue(m, pos, k) - getFunctionValue(m, pos, this->mGridDelimiters.size())) / (getGridValue(next_pos, k) - getGridValue(pos, k)));
								}
							}
							pos[0]++;
						}
						pos[1]++;
						pos[0] = 0;
					}
					pos[2]++;
					pos[1] = 0;
					pos[0] = 0;
				}
				pos[3]++;
				pos[2] = 0;
				pos[1] = 0;
				pos[0] = 0;
			}

		}
	}
};

// 5D interpolator
template <typename T> class Interpolator5D : public Interpolator<T> {
public:
	using Interpolator<T>::getFunctionValue;
	using Interpolator<T>::getGridValue;

	Interpolator5D() : Interpolator<T>() {}
	virtual ~Interpolator5D() {}

	virtual void calculateGradients() {

		for (std::size_t m = 0; m < this->mFunctions.size(); m++) {

			std::vector<std::size_t> pos(5, 0);
			for (std::size_t n = 0; n < this->mGridDelimiters[4]; n++) {
				for (std::size_t m = 0; m < this->mGridDelimiters[3]; m++) {
					for (std::size_t l = 0; l < this->mGridDelimiters[2]; l++) {
						for (std::size_t i = 0; i < this->mGridDelimiters[1]; i++) {
							for (std::size_t j = 0; j < this->mGridDelimiters[0]; j++) {
								for (std::size_t k = 0; k < this->mGridDelimiters.size(); k++) {
									std::vector<std::size_t> next_pos = pos;
									next_pos[k]++;
									if ((k == 4 && n == this->mGridDelimiters[4] - 1) || (k == 3 && m == this->mGridDelimiters[3] - 1) ||
										(k == 2 && l == this->mGridDelimiters[2] - 1) || (k == 1 && i == this->mGridDelimiters[1] - 1) || 
										(k == 0 && j == this->mGridDelimiters[0] - 1)) {
										this->mGradients[m].push_back(getFunctionValue(m, pos, this->mGridDelimiters.size()));
									} else {
										this->mGradients[m].push_back((getFunctionValue(m, pos, k) - getFunctionValue(m, pos, this->mGridDelimiters.size())) / (getGridValue(next_pos, k) - getGridValue(pos, k)));
									}
								}
								pos[0]++;
							}
							pos[1]++;
							pos[0] = 0;
						}
						pos[2]++;
						pos[1] = 0;
						pos[0] = 0;
					}
					pos[3]++;
					pos[2] = 0;
					pos[1] = 0;
					pos[0] = 0;
				}
				pos[4]++;
				pos[3] = 0;
				pos[2] = 0;
				pos[1] = 0;
				pos[0] = 0;
			}

		}
	}
};

template <typename T> int Interpolator<T>::mIDGenerator = 0;

#endif